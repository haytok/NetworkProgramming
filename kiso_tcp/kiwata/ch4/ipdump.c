#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <stdbool.h>
#include <unistd.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>

#define MAX_SIZE 8192

void die(char *msg);
void print_ethernet(struct ether_header *eth);
void print_arp(struct ether_arp *arp);
char *ip_ttoa(int flag);
char *ip_ftoa(int flag);
void print_ip(struct ip *ip);
void print_icmp(struct icmp *icmp);
void print_udp(struct udphdr *udp);
void print_tcp(struct  tcphdr *tcp);
char *mac_ntoa(uint8_t *host);

int main(int argc, char **argv) {
    puts("ipdump");
    int s;

    // 第二引数が SOCK_RAW だと正常に動作しない。
    if ((s = socket(AF_INET, SOCK_PACKET, ntohs(ETH_P_ALL))) < 0) {
        die("socket");
    }

    while (true) {
        int nread;
        void *head;
        void *p0;
        char buf[MAX_SIZE];
        struct ether_header *eth; // net/ethernet.h に定義されている。
        struct ether_arp *arp;
        struct ip *ip;
        struct icmp *icmp;
        struct udphdr *udp;
        struct tcphdr *tcp;

        if ((nread = read(s, buf, MAX_SIZE)) < 0) {
            die("read");
        }
        p0 = head = buf;

        /* ヘッダの解析処理 */
        eth = (struct ether_header *) head;
        head += sizeof(struct ether_header);
        if (ntohs(eth->ether_type) == ETHERTYPE_ARP) {
            arp = (struct ether_arp *) head;
        } else if (ntohs(eth->ether_type) == ETHERTYPE_IP) {
            ip = (struct ip *) head;
            head += ((int) (ip->ip_hl) << 2);

            switch (ip->ip_p) {
                case IPPROTO_TCP:
                    tcp = (struct tcphdr *) head;
                    head += ((int) (tcp->th_off) << 2); // TCP が運んでいるデータがヘッダの先頭から数えてどこから始まるかを表す。
                    break;
                case IPPROTO_UDP:
                    udp = (struct udphdr *) head;
                    head += sizeof(struct udphdr);
                    break;
                case IPPROTO_ICMP:
                    icmp = (struct icmp *) head;
                    head = icmp->icmp_data;
                    break;
                default:
                    break;
            }
        }

        /* ヘッダを表示する */
        // print_ethernet(eth);
        if (ntohs(eth->ether_type) == ETHERTYPE_ARP) {
            // print_arp(arp);
        } else if (ntohs(eth->ether_type) == ETHERTYPE_IP) {
            // print_ip(ip);
            if (ip->ip_p == IPPROTO_TCP) {
                print_tcp(tcp);
            } else if (ip->ip_p == IPPROTO_UDP) {
                // print_udp(udp);
            } else if (ip->ip_p == IPPROTO_ICMP) {
                print_icmp(icmp);
            }
        }
    }
    close(s);
}

char *mac_ntoa(uint8_t *host) {
    #define MAX_MACADDRESS_LEN 50
    static char str[MAX_MACADDRESS_LEN]; // static 修飾子を付けないと、その値を return できない。

    snprintf(str, MAX_MACADDRESS_LEN, "%02x:%02x:%02x:%02x:%02x:%02x", host[0],
             host[1], host[2], host[3], host[4], host[5]);
    return str;
}

void print_ethernet(struct ether_header *eth) {
    int type = ntohs(eth->ether_type);

    if (type <= 1500) {
        printf("IEEE 802.3 Ethernet Frame:\n");
    } else {
        printf("Ethernet Frame:\n");
    }

    printf("+-------------------------+-------------------------"
         "+-------------------------+\n");
    printf("| Destination MAC Address %17s|\n", mac_ntoa(eth->ether_dhost));
    printf("| Source MAC Address %17s|\n", mac_ntoa(eth->ether_shost));
    printf("+-------------------------+-------------------------"
         "+-------------------------+\n");
    if (type < 1500) {
        printf("| Length:            %5u|\n", type);
    } else {
        printf("| Ethernet Type:    0x%04x|\n", type);
    }
    printf("+-------------------------+\n");
}


void print_arp(struct ether_arp *arp) {
    int op = ntohs(arp->arp_op);

    puts("Protocol: ARP");
    printf("+-------------------------+-------------------------+\n");
    printf("| Hard: %s | Protocol: %s |\n", ntohs(arp->arp_hrd) == ARPHRD_ETHER ? "(Ethernet)" : "(Not Ether)",
           ntohs(arp->arp_pro) == ETHERTYPE_IP ? "(IP)" : "(Not IP");
    printf("| Hard Len %3u | Protocol Len %2u | Op %4d%16s |\n", ntohs(arp->arp_hln),
           ntohs(arp->arp_pln), op, op == ARPOP_REQUEST ? "(ARP Request)" : (op == ARPOP_REPLY ? "(ARP Request)" : "Undefined") );
    printf("| Source MAC Address %17s|\n", mac_ntoa(arp->arp_sha));
    printf("| Source IP Address %s |\n", inet_ntoa(*(struct in_addr *) &arp->arp_spa));
    printf("| Destination MAC Address %17s|\n", mac_ntoa(arp->arp_tha));
    printf("| Destination IP Address %s |\n", inet_ntoa(*(struct in_addr *) &arp->arp_tpa));
    printf("+-------------------------+-------------------------+\n");
}

char *ip_ttoa(int flag) {
    static int f[] = {'1', '1', '1', 'D', 'T', 'R', 'C', 'X'};
    #define TOS_MAX (sizeof f / sizeof f[0])
    static char str[TOS_MAX + 1];
    int i;

    for (i = 0; i < TOS_MAX; i++) {
        if ((flag << i) & IP_RF) {
            str[i] = f[i];
        } else {
            str[i] = '0';
        }
    }
    str[i] = '\0';

    return str;
}

char *ip_ftoa(int flag) {
    static int f[] = {'R', 'D', 'M'};
    #define TOS_MAX (sizeof f / sizeof f[0])
    static char str[TOS_MAX + 1];
    int i;

    for (i = 0; i < TOS_MAX; i++) {
        if ((flag << i) & IP_RF) {
            str[i] = f[i];
        } else {
            str[i] = '0';
        }
    }
    str[i] = '\0';

    return str;
}

void print_ip(struct ip *ip) {
    puts("Protocol: IP");
    printf("+-----+------+------------+-------------------------+\n");
    printf("| V %u | L %u | T %s |\n", ip->ip_v, ip->ip_hl, ip_ttoa(ip->ip_tos));
    printf("| ID  %u | Flags %3s | Fragment Offset %u | \n", ntohs(ip->ip_id), ip_ftoa(ntohs(ip->ip_off)), ntohs(ip->ip_off) & IP_OFFMASK);
    printf("| TTL %3u | Protocol %3u | Checksum %5u |\n", ip->ip_ttl, ip->ip_p, ntohs(ip->ip_sum));
    printf("| Source IP Address %s |\n", inet_ntoa(*(struct in_addr *) &ip->ip_src));
    printf("| Destination IP Address %s |\n", inet_ntoa(*(struct in_addr *) &ip->ip_dst));
    printf("+---------------------------------------------------+\n");
}

void print_icmp(struct icmp *icmp) {
    puts("Protocol: ICMP");
}

void print_udp(struct udphdr *udp) {
    puts("Protocol: UDP");
    printf("+-------------------------+-------------------------+\n");
    printf("| Source Port %5u | Dest Port %5u |\n", ntohs(udp->uh_sport), ntohs(udp->uh_dport));
    printf("| Length %5u | Checksum %5u |\n", ntohs(udp->uh_ulen), ntohs(udp->uh_ulen));
    printf("+-------------------------+-------------------------+\n");
}

void print_tcp(struct  tcphdr *tcp) {
    puts("Protocol: TCP");
    printf("+-------------------------+-------------------------+\n");
    printf("||\n");
    printf("+-------------------------+-------------------------+\n");
}

void die(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}
