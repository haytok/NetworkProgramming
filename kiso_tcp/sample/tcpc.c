#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#define BUFSIZE      1000
#define DEFAULT_PORT 5320

int main(int argc, char *argv[])
{
    struct sockaddr_in server;
    char buf[BUFSIZE];
    int s, c, port;
    unsigned long address;

    if (argc !=2 && argc != 3) {
        fprintf(stderr, "Usage�F %s  hostname  [port]\n", argv[0]);
        exit(1);
    }

    if ((address = inet_addr(argv[1])) == INADDR_NONE) {
        struct hostent *he;
        if ((he = gethostbyname(argv[1])) == NULL) {
            herror("gethostbyname");
            exit(1);
        }
        memcpy((char *) &address, (char *)he->h_addr, he->h_length);
    }

    if (argc == 3) {
        if ((port = atoi(argv[2])) == 0) {
            struct servent *se;
            if ((se = getservbyname(argv[2], "tcp")) != NULL)
                port = (int)ntohs((u_short)se->s_port);
            else {
                fprintf(stderr, "getservbyname error\n");
                exit(1);
            }
        }
    } else
        port = DEFAULT_PORT;

    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    memset((char *) &server, 0, sizeof(server));
    server.sin_family      = AF_INET;
    server.sin_addr.s_addr = address;
    server.sin_port        = htons(port);

    if (connect(s, (struct sockaddr *) &server, sizeof(server)) < 0) {
        perror("connect");
        exit(1);
    }

    while ((c=getchar()) != EOF) {
        if (c == '\n') {
            send(s, "\r\n", 2, 0);
            while (1) {
                recv(s, buf, 1, 0);
                if (buf[0] == '\r' || buf[0] == '\n') {
                    putchar('\n');
                    recv(s, &buf[1], 1, 0);
                    if (buf[0] == '\r' && buf[1] == '\n')
                        break;
                    else {
                        putchar('\n');
                    }
                }
                putchar(buf[0]);
            }
        } else {
            buf[0] = (char)c;
            send(s, buf, 1, 0);
        }
    }

    close(s);
    exit(0);
}
