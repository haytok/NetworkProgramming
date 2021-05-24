# 概要

- 基礎からわかるTCP/IP ネットワーク実験プログラミング（第2版）の 5 章を読んで、調査・検証したり、実装して得た知見をまとめていく。
- 実装の方針としては、ホストをスキャンするための最小のプログラムである。

## scanhost.c に関して

- このプログラムは、ホストスキャンをするプログラムである。

### 実装方針

- イメージとしては、`ping コマンド` である。`ICMP ECHO 要求メッセージ` を活用してホストが稼働しているかを確認する。

- 入力
  - ある範囲の IP アドレス

- 出力
  - IP アドレスが存在するかを確認する

- ICMP のデータを作成 (今回は、送信時間を入れる。)

- ICMP のヘッダを作成

- sendto, recvfrom を呼び出して、応答メッセージがあるかを確認する。

> C 言語では、IP 層以下のヘッダ情報を操作するためには Raw Socket を使う必要がある。
> 「raw ソケット」は、ICMP へのアクセスを提供します。raw ソケットは、ネットワーキングスタックによって直接サポートされない IP ベースのほかのプロトコルへのアクセスも提供します。このタイプのソケットは、通常、データグラム型ですが、実際の特性はプロトコルが提供するインタフェースに依存します。raw ソケットは、ほとんどのアプリケーションには使用されません。raw ソケットは、新しい通信プロトコルの開発をサポートしたり、既存プロトコルの難解な機能にアクセスしたりするために提供されています。raw ソケットを使用できるのは、スーパーユーザープロセスだけです。ソケットタイプは SOCK_RAW です。
> raw ソケットを使うと、新しい IPv4 プロトコルをユーザー空間で 実装できるようになる。 raw ソケットは、`リンクレベルヘッダーを 含まない` raw データグラムの送受信ができる。

### socket() システムコールの第三引数 protocol に代入する関数に関して

#### 背景と目的

- `scanhost.c` のプログラムを実装する際に、socket() の第三引数に入れる値を迷ったので、ログに残す。

#### 方法と結論

- `/usr/include/netinet/in.h` に記述されている。`TCP` や `UDP` のクライアントとサーバを socket() を使って実装する際には、`protocotl` には 0 を入れるものだと思っていた。しかし、この 0 という値は `IPPROTO_IP` のことだと思う。すなはち、protocol には、socket() の上位層のプロトコルの値を入れれば良い。以下の man に記述されていることが参考になる。

- man 2 socket

> The protocol specifies a particular protocol to be used with the socket.  Normally only a single protocol exists to support a particular  socket  type  within  a given  protocol family, in which case protocol can be specified as 0.  However, it is possible that many protocols may exist, in which case a particular protocol must be specified in this manner.  The protocol number to use is specific to the “communication domain” in which communication  is  to  take  place;  see  protocols(5).  See getprotoent(3) on how to map protocol name strings to protocol numbers.

- man 5 protocols

```
NAME
       protocols - protocols definition file

DESCRIPTION
       This file is a plain ASCII file, describing the various DARPA internet protocols that are available from the TCP/IP subsystem.  It should be consulted instead of
       using the numbers in the ARPA include files, or, even worse, just guessing them.  These numbers will occur in the protocol field of any IP header.

       Keep this file untouched since changes would result in incorrect IP packages.  Protocol numbers and names are specified by the IANA  (Internet  Assigned  Numbers
       Authority).

       Each line is of the following format:

              protocol number aliases ...

       where the fields are delimited by spaces or tabs.  Empty lines are ignored.  If a line contains a hash mark (#), the hash mark and the part of the line following
       it are ignored.

       The field descriptions are:

       protocol
              the native name for the protocol.  For example ip, tcp, or udp.

       number the official number for this protocol as it will appear within the IP header.

       aliases
              optional aliases for the protocol.

       This file might be distributed over a network using a network-wide naming service like Yellow Pages/NIS or BIND/Hesiod.

FILES
       /etc/protocols
              The protocols definition file.
```

- `/etc/protocol` にもプロトコル番号が定義されている。おそらく、Kernel がファイルシステムの設定ファイルの値を読み込み、ヘッダファイルに設定していると推測できる。だが、キチンと調査していないので、今後の課題としたい。
- また、5 番はファイルフォーマットの説明を表す。

### sockaddr_in の定義箇所を探す。

- まず、 `find /usr/include/net* -type f -print | xargs grep struct sockaddr_in {` を実行する。
- `/usr/include/netdb.h` が怪しそうなので、見に行く。しかし、定義が見つからないので、Include ファイルを参考に、次は `#include <netinet/in.h>` を見に行く。
- `/usr/include/netinet/in.h` を見ると、`struct sockaddr_in` が見つかる。そして、そのメンバに `struct in_addr sin_addr;` があるのを確認できる。
- また、`inet_ntoa` の引数は、`struct in_addr in` であるから、`sockaddr_in` オブジェクトの `sin_addr` を `inet_ntoa` に引き渡すと、IP アドレスを取得できる。

### 実装していないこと

- Broadcast パケットは受信する設定にしていない。
- RTT を計算し表示する機能は実装していない。

### 参考

- [RAW](https://linuxjm.osdn.jp/html/LDP_man-pages/man7/raw.7.html)
- [ソケットの概要](https://docs.oracle.com/cd/E19253-01/819-0392/sockets-42163/index.html)

## scanport_tcp.c に関して

- 特になし。

## scanport_udp.c に関して

-
