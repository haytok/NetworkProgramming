# 概要

- 基礎からわかるTCP/IP ネットワーク実験プログラミング（第2版）の 7 章を読んで、調査・検証したり、実装して得た知見をまとめていく。

- redirect のプログラムに関しては、新しい Linux で紹介されているプログラムが動作しないので、コンパイルが通れば最低限は大丈夫とみなす。

## redirect.c に関して

### プログラムの引数の調査のフロー

- man でシステムコールの include ファイルを調べる。
- その結果に従って `/usr/include/hogehoge` とそこで include されているファイルを逐一確認しに行く。

- `void make_udp_header((struct udphdr *) udp);` この記述をすると、型指定子が必要ですのエラーが生じる。

## scanroute.c に関して

- `socket()` の第三引数は、作成したいパケットの種類を書く？
- `ICMP_TIMXCEED_INTRANS` は、`ttl==0 in transit` とコメントされている。

- IP をドメインに変換する処理は実装していない。
- 往復時間を計算するロジックは実装していない。
