#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <stdbool.h>

#define DEFAULT_PORT 5320
#define DEFAULT_DEST_ADDRESS "192.168.11.16"
#define BUF_SIZE 8192

void die(char *msg);

void die(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

/*
クライアント側のプログラムを実装する
*/
int main(int argc, char **argv) {
    int s;
    struct sockaddr_in server;
    char *address;
    unsigned long dest_ip;
    int port;
    char send_buf[BUF_SIZE];

    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        die("socket");
    }

    address = "192.168.11.16";  // DEFAULT_DEST_ADDRESS;
    dest_ip = inet_addr(address);
    port = DEFAULT_PORT;

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = dest_ip;
    server.sin_port = htons(port);
    printf("Before connect.\n");
    if (connect(s, (struct sockaddr *) &server, sizeof(server)) < 0) {
        die("connect");
    }

    printf("Connected\n");

    while (true) {
        char *buf = "Hayato Kiwata\n";
        if (fgets(send_buf, BUF_SIZE - 2, stdin) == NULL) {
            break;
        }
        if (send(s, send_buf, strlen(send_buf), 0) < 0) {
            die("send");
        }


        char recv_buf[BUF_SIZE];
        while (true) {
            int i;
            recv_buf[0] = '\0';
            for (i = 0; i < BUF_SIZE - 1; i++) {
                if (recv(s, &recv_buf[i], 1, 0) < 0) {
                    goto exit_loop;
                }
                if (recv_buf[i] == '\n') {
                    break;
                }
            }
            printf("In Client, %s\n", recv_buf);
            break;
        }

        printf("Received message is %s\n", recv_buf);
    }
    exit_loop:
        printf("End");
        close(s);

        return 0;
}
