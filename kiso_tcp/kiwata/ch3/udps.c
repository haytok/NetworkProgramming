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

#define DEFAULT_PORT 5320
#define BUF_SIZE 32768

void die(char *msg);

void die(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

// socket() -> bind() -> recvfrom() -> sendto() の流れで実装していく。
int main(int argc, char **argv) {
    int s;
    struct sockaddr_in server;
    int port;
    int rn;
    char rcv_buf[BUF_SIZE];
    struct sockaddr_in client;
    int len;

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        die("socket");
    }

    port = DEFAULT_PORT;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);
    if (bind(s, (struct sockaddr *) &server, sizeof(server)) < 0) {
        die("bind");
    }

    while (
        (rn = recvfrom(s, rcv_buf, BUF_SIZE - 1, 0, (struct sockaddr *) &client, (len = sizeof(client), &len))) >= 0
    ) {
        rcv_buf[rn] = '\0';
        printf("rcv_buf %s\n", rcv_buf);
    }
    close(s);

    return 0;
}
