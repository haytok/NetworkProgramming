# 概要

- 基礎からわかるTCP/IP ネットワーク実験プログラミング（第2版）の 2 章を読んで、調査・検証したり、実装して得た知見をまとめていく。

## Ethernet のフレームフォーマットと構造体の定義

- `/usr/include/net/ethernet.h` で構造体を確認する。

- `ETH_ALEN` の定義を確認したいので、`/usr/include/sys/types.h` を確認してみる。しかし、定義は存在しない。

- 次に、`#include <linux/if_ether.h>` の実体を確認したい。パスがわからなかったので、`find /usr/include/ | grep if_ehter.h` のコマンドを実行するした。そうすると見つかった。

- `/usr/include/net/ethernet.h`

```c
struct ether_header
{
    uint8_t ether_dhost[ETH_ALEN];
    uint8_t ether_shost[ETH_ALEN];
    uint16_t ether_type;
} __attribute__ ((__packed__));

/* Ethernet protocol IDs*/
#define ETHERTYPE_IP 0x0800
#define ETHERTYPE_ARP 0x0806
```

- `/usr/include/linux/if_ether.h`

```c
#define ETH_ALEN 6 // Octets in one ethernet addr
```

- また、FCS の計算はハードウェアで自動的に計算されるので、プログラムから直接計算する必要はない。
- Ethernet のフレームを受け取った時にまず、自信の NIC の MAC アドレスと`ether_dhost` を比較する。このアドレスが自分宛てであれば、`ether_type` をもとに上位層モジュールを特定し、そのモジュールにデータを引き渡す。そうでなければ、フレームを破棄する処理を行う。

## ARP のパケットフォーマット

- ARP ヘッダ構造体 (arphdr) は、`/usr/include/net/if_arp.h` で定義されている。ここには、データリンクによらない共通部分が定義されている。

```c
struct arphdr {
    unsigned short int ar_hdr;
    unsigned short int ar_pro;
    unsigned char ar_hln;
    unsigned char ar_plen;
    unsigned short int ar_op; // ARP 要求か ARP 応答か
};

#define ARPOP_REQUEST 1
#define ARPOP_REPLY 2
```

- Ethernet の ARP パケット構造体 (ether_arp) は、`/usr/include/netinet/if_ether.h` で定義されている。ここには、Ethernet 上で使用される ARP パケットが定義されている。

```c
struct ether_arp {
    strut arphdr ea_hdr;
    uint8_t arp_sha[ETH_ALEN];
    uint8_t arp_spa[4];
    uint8_t arp_tha[ETH_ALEN];
    uint8_t apr_tpa[4];
};

#define arp_hdr ea_hdr.ar_hdr;
#define arp_pro ea_hdr.ar_pro;
#define arp_hln ea_hdr.ar_hln;
#define arp_plen ea_hdr.ar_plen;
#define arp_op ea_hdr.ar_op;
```

- 例えば、ARP 要求パケットをブロードキャストする際に、ARP パケットに格納されている値の例
  - 本の p.54 の図に沿う。
  - ホスト B (IP アドレス: 192.168.13.23, MAC アドレス: 00:50:04:22:88:D4) が IP アドレスが 192.168.3.1 の MAC アドレスを求める。

```c
struct ehter_arp e_a;
memset(&e_a, 0, sizeof(e_a));
e_a.arp_hdr = ARPHRD_ETHER;
e_a.arp_pro = ; // プロトコルアドレスの種類で、Ethernet のタイプフィールドに定義されている値と同じ値を入れる。
e_a.arp_hln = ETHERTYPE_IP;
e_a.arp_plen = 4;
e_a.arp_op = ARPOP_REQUEST; // ARP 要求 or 応答
e_a.arp_sha = 00:50:04:22:88:D4;
e_a.arp_spa = 192.168.13.23;
e_a.arp_tha = ff:ff:ff:ff:ff:ff;
e_a.arp_tpa = 192.168.3.1;
```

- 上述の条件の際、Ethernet ヘッダが以下のようになる。

```c
struct ether_header e_h;
memset(&e_h, 0, sizeof(e_h));
e_h.ehter_dhost = ff:ff:ff:ff:ff:ff;
e_h.ehter_shost = 00:50:04:22:88:D4;
e_h.ehter_type = ETHERTYPE_ARP;
```

## IP ヘッダとヘッダ構造体について

## ICMP パケット構造体

## UDP ヘッダとヘッダ構造体について

## TCP ヘッダとヘッダ構造体について
