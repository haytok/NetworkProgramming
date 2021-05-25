#include <stdio.h>
#include <sys/types.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <sys/time.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>

#define BUF_MAX 8192

void die(char *msg);
bool udp_port_scan(struct sockaddr_in server, int send_s, int recv_s);

enum {
    CMD_NAME,
    DEST_IP,
    START_PORT,
    END_PORT
};

int main(int argc, char **argv) {
    puts("scanport_udp");
    if (argc != 4) {
        die("input error");
    }

    in_addr_t dest_ip;
    int start_port;
    int end_port;
    int send_s;
    int recv_s;

    dest_ip = inet_addr(argv[DEST_IP]);
    start_port = atoi(argv[START_PORT]);
    end_port = atoi(argv[END_PORT]);

    if ((send_s = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) < 0) {
        die("send socket");
    }
    if ((recv_s = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        die("recv socket");
    }

    for (int port = start_port; port <= end_port; port++) {
        // printf("Scan port %d\n", port);

        struct sockaddr_in server;

        memset(&server, 0, sizeof(server));
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = dest_ip;
        // htons でネットワークバイトオーダに変換しないと、正常に動作しない。
        server.sin_port = htons(port);

        if (udp_port_scan(server, send_s, recv_s)) {
            struct servent *sv;

            // 第一引数を int 型から htons を使ってキャストする必要がある。
            // 第二引数は、NULL でも正常に動作する。
            sv = getservbyport(htons(port), NULL);

            // サービス名が存在しない時は、NULL が返るので NULL チェックが必要。
            printf("Port %d -> %s\n", port, (sv != NULL) ? sv->s_name : "uknown");
        }
    }
    close(send_s);
    close(recv_s);
}

bool udp_port_scan(struct sockaddr_in server, int send_s, int recv_s) {
    static bool can_connect;
    fd_set rfds;
    struct timeval tv;
    char buf[BUF_MAX];

    FD_ZERO(&rfds);
    FD_SET(recv_s, &rfds);

    tv.tv_sec = 1;
    tv.tv_usec = 0;

    sendto(send_s, NULL, 0, 0, (struct sockaddr *)&server, sizeof(server));

    while (true) {
        if (select(recv_s + 1, &rfds, NULL, NULL, &tv) > 0) {
            struct ip *ip_1;
            int ip_1_len;
            struct icmp *icmp;
            struct ip *ip_2;
            int ip_2_len;
            struct udphdr *udp;

            // 20 + 8 + 20 + 8
            if (recvfrom(recv_s, buf, BUF_MAX, 0, NULL, NULL) < 56) {
                continue;
            }
            ip_1 = (struct ip *)buf;
            ip_1_len = ip_1->ip_hl << 2;
            icmp = (struct icmp *)((char *)buf + ip_1_len);
            ip_2 = (struct ip *)icmp->icmp_data;
            ip_2_len = ip_2->ip_hl << 2;
            udp = (struct udphdr *)((char *)ip_2 + ip_2_len);

            // ICMP 到達不能であるから、サービスは存在しない。
            if ((ip_1->ip_src.s_addr == server.sin_addr.s_addr)
                && (icmp->icmp_type == ICMP_UNREACH)
                && (icmp->icmp_code == ICMP_UNREACH_PORT)
                && (ip_2->ip_p == IPPROTO_UDP)
                && (udp->uh_dport == server.sin_port)
            ) {
                break;
            }
        } else {
            return true;
        }
    }
    return false;
}

void die(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}
