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

## IP ヘッダとヘッダ構造体について

## ICMP パケット構造体

## UDP ヘッダとヘッダ構造体について

## TCP ヘッダとヘッダ構造体について
