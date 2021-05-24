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
#include <netinet/ip_icmp.h>

#define PACKET_LEN 72
#define BUF_SIZE 4096

void die(char *msg);
void make_icmp8_packet(struct icmp *icmp, int len, int n);
u_int16_t checksum(u_int16_t *data, int len);

void die(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

enum {
    CMD_NAME,
    START_IP,
    END_IP
};

int main(int argc, char **argv) {
    puts("scanhost");

    int s;
    int start_ip;
    int end_ip;
    struct sockaddr_in server;

    if (argc != 3) {
        die("input error");
    }
    start_ip = htonl(inet_addr(argv[START_IP]));
    end_ip = htonl(inet_addr(argv[END_IP]));

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;

    if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        die("socket");
    }

    for (int dest_ip = start_ip; dest_ip <= end_ip; dest_ip++) {

        server.sin_addr.s_addr = ntohl(dest_ip);

        // 3 回までは再送する。
        for (int i = 0; i < 3; i++) {
            // ボディを作成する
            char send_buf[PACKET_LEN];
            char recv_buf[BUF_SIZE];
            struct timeval tv;
            struct ip *ip;
            int ip_hlen;

            // printf("scan %s (%d)\n", inet_ntoa(server.sin_addr), i);
            fflush(stdout);

            make_icmp8_packet((struct icmp *)send_buf, PACKET_LEN, i);

            tv.tv_sec = 0;
            tv.tv_usec = 200 * 1000;

            if (sendto(s, (char *)&send_buf, PACKET_LEN, 0, (struct sockaddr *) &server, sizeof(server)) < 0) {
                die("sendto");
            }
            while (true) {
                puts("In while");
                fd_set rfds;
                struct icmp *icmp;

                FD_ZERO(&rfds);
                FD_SET(s, &rfds);
                if (select(s + 1, &rfds, NULL, NULL, &tv) <= 0) {
                    break;
                }

                if (recvfrom(s, recv_buf, BUF_SIZE, 0, NULL, NULL) < 0) {
                    die("recvfrom");
                }

                // 受信したパケットを処理
                ip = (struct ip *)recv_buf;
                ip_hlen = ip->ip_hl << 2;
                if (ip->ip_src.s_addr == server.sin_addr.s_addr) {
                    icmp = (struct icmp *)(recv_buf + ip_hlen);

                    if (icmp->icmp_type == ICMP_ECHOREPLY) {
                        printf("IP Address %s\n", inet_ntoa(*(struct in_addr *) &(ip->ip_src.s_addr)));
                        goto exit_loop;
                    }
                }
            }
        }
        exit_loop:;
    }
    close(s);
}

void make_icmp8_packet(struct icmp *icmp, int len, int n) {
    // 初期化
    memset(icmp, 0, len);

    gettimeofday((struct timeval *)icmp->icmp_data, (struct timezone *)0);

    icmp->icmp_type = ICMP_ECHO;
    icmp->icmp_code = 0;
    icmp->icmp_id = 0;
    icmp->icmp_seq = n;
    icmp->icmp_cksum = 0;
    icmp->icmp_cksum = checksum((uint16_t *)icmp, len);
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
