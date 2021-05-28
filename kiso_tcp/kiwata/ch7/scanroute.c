#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/in_systm.h>
#include <stdbool.h>
#include <sys/time.h>

#define BUF_SIZE 1024

void die(char *msg);
void make_udp_header(struct udphdr *udp);
void make_ip_header(struct ip *ip, int target_ip, int dest_ip, int iplen);
u_int16_t checksum(u_int16_t *data, int len);

enum {
    CMD_NAME,
    DEST_IP
};

struct packet_udp {
    struct ip ip;
    struct udphdr udp;
};

int main(int argc, char **argv) {
    puts("scanroute");

    if (argc != 2) {
        die("input error");
    }

    struct sockaddr_in server;
    int send_s;
    int on = 1;
    int recv_s;
    struct packet_udp send_packet;
    int len;
    struct ip *ip;
    int ihlen;
    struct icmp *icmp;

    // UDP で sendto する際に必要になる
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[DEST_IP]);
    // 本来なら、ドメイン名が来た時の条件分岐をする必要があるが、一旦飛ばす。

    if ((send_s = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) < 0) {
        die("socket");
    }
    // IP ヘッダを自前で作成するオプション
    if (setsockopt(send_s, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0) {
        die("setsockopt");
    }

    if ((recv_s = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        die("socket");
    }


    // 送信する際のヘッダ (UDP, IP ヘッダ) を作成する
    len = sizeof(struct packet_udp);
    memset(&send_packet, 0, len);
    make_udp_header(&(send_packet.udp));
    make_ip_header(&(send_packet.ip), 0, server.sin_addr.s_addr, len);
    puts("Hello");

    puts("traceroute");
    for (int ttl = 1; ttl <= 64; ttl++) {
        printf("ttl %d\n", ttl);

        int i;

        send_packet.ip.ip_ttl = ttl;

        for (i = 0; i < 3; i++) {
            if (sendto(send_s, (char *) &send_packet, len, 0, (struct sockaddr *) &server, sizeof(server)) < 0) {
                die("sendto");
            }
            struct timeval tv;

            tv.tv_sec = 3;
            tv.tv_usec = 0;

            while (true) {
                fd_set rfds;
                int n;
                char buf[BUF_SIZE];

                FD_ZERO(&rfds);
                FD_SET(recv_s, &rfds);

                if ((n = select(recv_s + 1, &rfds, NULL, NULL, &tv)) < 0) {
                    die("select");
                }
                if (n == 0) {
                    puts("???");
                    fflush(stdout);
                    break;
                }

                if (recvfrom(recv_s, buf, BUF_SIZE, 0, NULL, NULL) < 0) {
                    die("recvfrom");
                }

                // 受信したパケットの解析処理

                ip = (struct ip *)buf;
                ihlen = ip->ip_hl << 2;
                icmp = (struct icmp *)(buf + ihlen);
                
                if ((icmp->icmp_type == ICMP_TIME_EXCEEDED &&
                     icmp->icmp_code == ICMP_TIMXCEED_INTRANS) ||
                    icmp->icmp_type == ICMP_DEST_UNREACH) {
                    goto exit_loop;
                }
            }
        }
        exit_loop:
        if (i < 3) {
            struct in_addr ip_address;
            char host_ip[INET_ADDRSTRLEN];

            snprintf(host_ip, INET_ADDRSTRLEN, "%s",
                     inet_ntoa(*(struct in_addr *)&(ip->ip_src.s_addr)));

            printf("%s\n", host_ip);

            if (icmp->icmp_type == ICMP_UNREACH) {
                switch (icmp->icmp_code) {
                    case ICMP_UNREACH_PORT:
                        puts("Reach!");
                        break;
                    case ICMP_UNREACH_HOST:
                        puts("Host Unreachable!");
                        break;
                    case ICMP_UNREACH_NET:
                        puts("Network Unreachable!");
                        break;
                }
                goto end_program;
            }
        }
        }
    end_program:

    close(send_s);
    close(recv_s);
}

void make_udp_header(struct udphdr *udp) {
    udp->uh_sport = htons(33434);
    udp->uh_ulen = htons((uint16_t)sizeof(struct udphdr));
    udp->uh_dport = htons(33434);
    udp->uh_sum = htons(0);
}

void make_ip_header(struct ip *ip, int target_ip, int dest_ip, int iplen) {
    memset(ip, 0, sizeof(ip));

    ip->ip_v = IPVERSION;
    ip->ip_hl = sizeof(struct ip) >> 2;
    ip->ip_id = htons(0);
    ip->ip_off = htons(0);

    ip->ip_ttl = 64;
    ip->ip_p = IPPROTO_UDP;
    ip->ip_src.s_addr = target_ip;
    ip->ip_dst.s_addr = dest_ip;

    ip->ip_sum = 0;
}

u_int16_t checksum(u_int16_t *data, int len) {
    u_int32_t sum = 0;

    for (; len > 1; len -= 2) {
        sum += *data++;

        if (sum & 0x80000000) {
            sum = (sum & 0xffff) + (sum >> 16);
        }
    }

    if (len == 1) {
        u_int16_t i = 0;
        *(u_char *) (&i) = *(u_char *) data;
        sum += i;
    }

    while (sum >> 16) {
        sum = (sum & 0xffff) + (sum >> 16);
    }

    return (sum == 0xffff) ? sum : ~sum;
}

void die(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}
