# 概要

- 基礎からわかるTCP/IP ネットワーク実験プログラミング（第2版）の 4 章を読んで、調査・検証したり、実装して得た知見をまとめていく。

## Linux のデータリンクアクセスインターフェース

- Linux の場合には、ソケットを利用してデータリンクにアクセスすることができる。このため、通常のソケットシステムコールをそのまま利用することができる。例えば、Ethernet ヘッダを直接操作してパケットを送受信したい場合には、ソケットをオープンするときに次のように書きます。

```c
s = socket(PF_PACKET, SOCK_PACKET, htons(ETH_P_ALL));
```

- IP パケットだけをモニタリングしたい場合には、以下のように書けば良い。

```c
s = socket(AF_INET, SOCK_PACKET, htons(ETH_P_ALLL));
```

## 各パケットのヘッダが格納されているディレクトリ

- `/usr/include/netinet/` 配下に格納されている。

## 雑多なメモ

- ヘッダの構造体のメンバの型が `unsigned short` などの時は、`ntos` で変換する必要がある。
- 何かのマクロや型を調べる時に役立つコマンド (参考: [【find・grep】特定の文字列を含むファイルのリストを取得する方法。](https://qiita.com/pokari_dz/items/0f14a21e3ca3df025d21))

```bash
find /usr/include/ -type f -print | xargs <キーワード>
```
