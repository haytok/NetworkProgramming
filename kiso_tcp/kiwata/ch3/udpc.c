#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>

#define DEFAULT_PORT 5320
#define BUF_SIZE 32768

void die(char *msg);

void die(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

// socket() -> sendto -> recvfrom の流れで実装していく。
int main(int argc, char **argv) {
    int port = DEFAULT_PORT;
    char *address = "192.168.11.16";

    // 人間が読みやすい IP アドレスからネットワークバイトオーダに変換する。
    in_addr_t dest_ip;
    if ((dest_ip = inet_addr(address)) == INADDR_NONE) {
        struct hostent *he;
        he = gethostbyname(address);
    }

    int s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        die("socket");
    }

    while (true) {
        // bind は必要ない。
        // 理由は、socket にサーバーのアドレスやポート番号などの情報を能動的に紐付ける必要が無いからである。
        // もう少し突っ込んだ解説は README.md に記述した。

        // man で確認
        // ssize_t sendto(
        //     int sockfd,
        //     const void *buf,
        //     size_t len,
        //     int flags,
        //     const struct sockaddr *dest_addr,
        //     socklen_t addrlen
        // )
        // /usr/include/netinet/in.h を確認する。
        // struct sockaddr_in {
        //     sa_family_t sin_family;
        //     in_port_t sin_port;
        //     struct in_addr sin_addr;
        //     // padding のメンバもある。
        // }
        struct sockaddr_in server;
        fd_set rfds;
        struct timeval tv;

        // select() に必要なオブジェクトを作成する。
        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);
        FD_SET(s, &rfds);

        tv.tv_sec = 600;
        tv.tv_usec = 0;

        if (select(s + 1, &rfds, NULL, NULL, &tv) < 0) {
            fprintf(stderr, "\nTime Out\n");
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &rfds)) {
            int readn;
            char buf[BUF_SIZE];
            char cmd[BUF_SIZE];
            if ((readn = read(STDIN_FILENO, buf, BUF_SIZE - 1)) <= 0) {
                break;
            }
            buf[readn] = '\0';
            sscanf(buf, "%s", cmd);
            if (strcmp(cmd, "quit") == 0) {
                break;
            }
            memset(&server, 0, sizeof(struct sockaddr_in));
            server.sin_family = AF_INET;
            server.sin_addr.s_addr = dest_ip;
            server.sin_port = htons(port);
            // sendto の第五引数のキャストを忘れずに。
            printf("In UDP Client, sendmsg %s\n", buf);
            if (sendto(s, buf, readn, 0, (struct sockaddr *) &server, sizeof(server)) < 0) {
                die("sendto");
            }
        }

        // man で確認
        // ssize_t recvfrom(
        //     int sockfd,
        //     void *buf,
        //     size_t len,
        //     int flags,
        //     struct sockaddr *src_addr,
        //     socklen_t *addrlen
        // )
        // src_addr が NULL 以外で、下層のプロトコルからメッセージの送信元アドレスがわかる場合、この送信元アドレスが src_addr が指すバッファに格納される。
        // この場合、addrlen は入出力両方の引数となる。
        // 呼び出し前に、呼び出し元は src_addr に割り当てたバッファで初期化しておくべきである。
        // 呼び出し元が送信アドレスを必要としない場合には、src_addr と addrlen には NULL を指定するべきである。
        if (FD_ISSET(s, &rfds)) {
            ssize_t n;
            char recv_buf[BUF_SIZE];
            int zero = 0;
            if ((n = recvfrom(s, recv_buf, BUF_SIZE, 0, (struct sockaddr *) 0, &zero)) < 0) {
                die("recvfrom");
            }
            recv_buf[n] = '\0';
            printf("In UDP Client, recv_buf %s\n", recv_buf);
        }
    }
    close(s);

    return 0;
}
