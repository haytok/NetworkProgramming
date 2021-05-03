#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#include <windows.h>
#include <winsock.h>
#define perror(_Z) fprintf(stderr, "%s error\n", _Z)
#define herror(_Z) fprintf(stderr, "%s error\n", _Z)
#define exit(_Z)   WSACleanup();exit(_Z)
#define close(_Z)  closesocket(_Z);
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#define BUFSIZE      1000
#define DEFAULT_PORT 5320

main(int argc, char *argv[])
{
    struct sockaddr_in server, client;
    char buf[BUFSIZE];
    int s, len, zero, port, n;
    unsigned long address;

#ifdef WIN32
    WSADATA wsaData;
    if (WSAStartup(0x0101, &wsaData) != 0) {
        fprintf(stderr, "Winsock Error\n");
        exit(1);
    }
#endif

    if (argc !=2 && argc != 3) {
        fprintf(stderr, "UsageF %s  hostname  [port]\n", argv[0]);
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
            if ((se = getservbyname(argv[2], "udp")) != NULL)
                port = (int)ntohs((u_short)se->s_port);
            else {
                fprintf(stderr, "getservbyname error\n");
                exit(1);
            }
        }
    } else
        port = DEFAULT_PORT;

    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    memset((char *) &client, 0, sizeof(client));
    client.sin_family      = AF_INET;
    client.sin_addr.s_addr = htonl(INADDR_ANY);
    client.sin_port        = htons(0);

    if (bind(s, (struct sockaddr *) &client, sizeof(client)) < 0) {
        perror("bind");
        exit(1);
    }

    memset((char *) &server, 0, sizeof(server));
    server.sin_family      = AF_INET;
    server.sin_addr.s_addr = address;
    server.sin_port        = htons(port);

    len = sizeof(server);
    zero = 0;
    while (fgets(buf, BUFSIZE, stdin) != NULL) {
        sendto(s, buf, strlen(buf), 0, (struct sockaddr *) &server, len);
        n = recvfrom(s, buf, BUFSIZE-1, 0, (struct sockaddr *)0, &zero);
        buf[n] = '\0';
        fputs(buf, stdout);
    }

    close(s);
    exit(0);
}
