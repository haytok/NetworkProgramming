#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    struct sockaddr_in client, server;
    int s, len, i, n, port, pn;
    char buf[BUFSIZE], ip[20];

#ifdef WIN32
    WSADATA wsaData;
    if (WSAStartup(0x0101, &wsaData) != 0) {
        fprintf(stderr, "Winsock Error\n");
        exit(1);
    }
#endif

    if (argc == 2) {
        if ((port = atoi(argv[1])) == 0) {
            struct servent *se;
            if ((se = getservbyname(argv[1], "udp")) != NULL)
                port = (int)ntohs((u_short)se->s_port);
            else {
                fprintf(stderr, "getservbyname error");
                exit(1);
            }
        }
    } else
        port = DEFAULT_PORT;

    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    memset((char *) &server, 0, sizeof(server));
    server.sin_family        = AF_INET;
    server.sin_addr.s_addr   = htonl(INADDR_ANY);
    server.sin_port          = htons(port);

    if (bind(s, (struct sockaddr *) &server, sizeof(server)) < 0) {
        perror("bind");
        exit(1);
    }

    len = sizeof(client);
        while ((n = recvfrom(s, buf, BUFSIZE-1, 0, (struct sockaddr *)
                        &client, &len)) > 0) {
                buf[n] = '\0';
                strcpy(ip, inet_ntoa(client.sin_addr));
	        pn = (int)ntohs(client.sin_port);
	        printf("Client IP Address = %s Port Number = %d\n", ip, pn);
                printf("receive \"%s\"\n",buf);
                fflush(stdout);
                for(i = 0; i < n; i++)
                        buf[i] = toupper(buf[i]);
                printf("send    \"%s\"\n", buf);
                fflush(stdout);
                sendto(s, buf, n, 0, (struct sockaddr *) &client, len);
        }

    close(s);
    exit(0);
}
