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
#include <netdb.h>

void die(char *msg);
bool tcp_port_scan(struct sockaddr_in server);
void make_icmp8_packet(struct icmp *icmp, int len, int n);
u_int16_t checksum(u_int16_t *data, int len);

enum {
    CMD_NAME,
    DEST_IP,
    START_PORT,
    END_PORT
};

int main(int argc, char **argv) {
    puts("scanport_tcp");
    if (argc != 4) {
        die("input error");
    }

    in_addr_t dest_ip;
    int start_port;
    int end_port;

    dest_ip = inet_addr(argv[DEST_IP]);
    start_port = atoi(argv[START_PORT]);
    end_port = atoi(argv[END_PORT]);

    for (int port = start_port; port <= end_port; port++) {
        // printf("Scan port %d\n", port);

        struct sockaddr_in server;

        memset(&server, 0, sizeof(server));
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = dest_ip;
        // htons でネットワークバイトオーダに変換しないと、正常に動作しない。
        server.sin_port = htons(port);

        if (tcp_port_scan(server)) {
            struct servent *sv;

            // 第一引数を int 型から htons を使ってキャストする必要がある。
            // 第二引数は、NULL でも正常に動作する。
            sv = getservbyport(htons(port), NULL);

            // サービス名が存在しない時は、NULL が返るので NULL チェックが必要。
            printf("Port %d -> %s\n", port, (sv != NULL) ? sv->s_name : "uknown");
        }
    }
}

bool tcp_port_scan(struct sockaddr_in server) {
    int s;
    bool can_connect;

    if ((s = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) < 0) {
        die("socket");
    }

    // server オブジェクトは connect する際に、キャストする必要がある。
    // したがって、関数の引数として切り出す際は、参照渡しではなく値渡しで渡す。
    if (connect(s, (struct sockaddr *) &server, sizeof(server)) < 0) {
        can_connect = false;
    } else {
        can_connect = true;
    }
    close(s);

    return can_connect;
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
