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
#define BUF_SIZE 8192

void die(char *msg);

void die(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

/*
サーバ側のプログラムを実装する
*/

int main(int argc, char **argv) {
    int s0;
    struct sockaddr_in server;
    int port;
    // accept で必要な変数を宣言
    int s;
    struct sockaddr_in client;
    unsigned int len;

    if ((s0 = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        die("socket");
    }

    port = DEFAULT_PORT;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);
    if (bind(s0, (struct sockaddr *) &server, sizeof(server)) < 0) {
        die("bind");
    }

    if (listen(s0, 5) < 0) {
        die("listen");
    }

    while (true) {
        printf("Before accept\n");
        len = sizeof(client);
        if ((s = accept(s0, (struct sockaddr *) &client, &len)) < 0) {
            die("accept");
        }

        printf("Connected from %s\n", inet_ntoa(client.sin_addr));

        while (true) {
            char recv_buf[BUF_SIZE];

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
            printf("Received message is %s\n", recv_buf);
        }
        exit_loop:
            printf("Connection closed\n");
            close(s);
    }
    close(s0);

    return 0;
}
