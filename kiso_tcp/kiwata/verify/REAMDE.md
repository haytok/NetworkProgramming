# 概要
- 基礎からわかるTCP/IP ネットワーク実験プログラミング（第2版）を読んで、調査・検証したり、実装して得た知見をまとめていく。

## C 言語における # と ## 演算子について
- `##` は、トークン連結演算子と呼ばれる。二つのトークンを結合して置き換えることができる。これは、通常のマクロと関数マクロで使用することができる。
- 例えば、`a ## 1` とすると、結合して `a1` に置き換えられる。

- `#` は、文字列化演算子と呼ばれる。マクロ関数で受け取った値に対して、`#` を付けることで、文字列つまりダブルクオテーションを付けた状態に置き換える。

### 検証プログラム
- `test_0.c`

### 参考
- [#と##演算子](http://wisdom.sakura.ne.jp/programming/c/c42.html)


## プリプロセッサを使ってマルチプラットフォーム対応を行う
- `#ifdef __linux` を使用してコンパイル時にプラットフォームによって条件分岐を行う。

### 検証プログラム
- `test_1.c`

### 参考
- [C言語でLinux、Solaris向け処理を#ifdefで分岐](https://mokky14.hatenablog.com/entry/2013/05/29/113244)
- [第14回 ヘッダファイルとプリプロセッサ指令](https://dev.grapecity.co.jp/support/powernews/column/clang/014/page02.htm)


## 3 章で作成する UDP Client のプログラムを実装する際に必要なシステムコール
- [ ] select() (今回のプログラムでは、UDP のクライアント側で呼出されている。)
- [ ] sendto()
- [ ] recvfrom()
- [ ] inet_addr()
- [ ] gethostbyname()
- [ ] getservbyname()
- [ ] ntohs()

### select()
- man を確認する。

```bash
SYNOPSIS
       /* According to POSIX.1-2001, POSIX.1-2008 */
       #include <sys/select.h>

       /* According to earlier standards */
       #include <sys/time.h>
       #include <sys/types.h>
       #include <unistd.h>

       int select(int nfds, fd_set *readfds, fd_set *writefds,
                  fd_set *exceptfds, struct timeval *timeout);

       void FD_CLR(int fd, fd_set *set);
       int  FD_ISSET(int fd, fd_set *set);
       void FD_SET(int fd, fd_set *set);
       void FD_ZERO(fd_set *set);

DESCRIPTION
       select()  and  pselect() allow a program to monitor multiple file descriptors, waiting until one or more of the file descriptors become "ready" for some class of
       I/O operation (e.g., input possible).  A file descriptor is considered ready if it is possible to perform a corresponding I/O operation  (e.g.,  read(2)  without
       blocking, or a sufficiently small write(2)).

```

- `fd_set` は、ファイルディスクリプタの集合を表す。
- `FD_ZERO(&fds)` は、初期化 (集合を空にする) するヘルパーマクロである。
- `FD_SET(fd, &fds)` は、fd を集合に加える。
- `FD_CLR(fd, &fds)` は、fd を集合から取り除く。
- `FD_ISSET(fd, &fds)` は、fd がその集合に含まれているかを確認する。

- `#define FD_SETSIZE 1024` のマクロがどこかで定義されている。

#### 参考
- [SELECT](https://linuxjm.osdn.jp/html/LDP_man-pages/man2/select.2.html)
- [[C言語] ライブラリの関数などメモ](https://qiita.com/edo_m18/items/7414028fd91269e5427d#select)

- [selectを使う](https://www.geekpage.jp/programming/linux-network/select.php)
- [システムプログラム（第8週）: select()による複数のクライアントに対するサービスの同時提供](http://www.coins.tsukuba.ac.jp/~syspro/2013/2013-06-05/echo-server-select.html)
- [C 言語で echo サーバを作ってみよう (2)](http://x68000.q-e-d.net/~68user/net/c-echo-2.html)

### sendto()
- man を確認する。
- `sendoto()` を呼び出す時に、キャストする `struct sockaddr` とは後述で説明している。
- もとの変数の型は、`struct sockaddr_in` である。
- この構造体を使って UDP で通信する際に必要なプロトコルファミリー、IP アドレス、ポート番号を設定する。

```bash
SYNOPSIS
       #include <sys/types.h>
       #include <sys/socket.h>

       ssize_t send(int sockfd, const void *buf, size_t len, int flags);

       ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
                      const struct sockaddr *dest_addr, socklen_t addrlen);

DESCRIPTION
       The system calls send(), sendto(), and sendmsg() are used to transmit a message to another socket.

RETURN VALUE
       On success, these calls return the number of bytes sent.  On error, -1 is returned, and errno is set appropriately.
```

- `/user/include/netinet/in.h` に `struct sockaddr_in` が定義されている。

```c
#include <bits/in.h>

typedef uint32_t in_addr_t;
typedef uint16_t in_port_t;

struct in_addr {
    in_addr_t s_addr;
};

struct sockaddr_in {
    // 2 Bytes
    __SOCKADDR_COMMON (sin_); // マクロで展開されて、sa_family_t sin_family に展開される。
    // 2 Bytes
    in_port_t sin_port; // port number
    // ? Bytes
    struct in_addr sin_addr; // address

    /* Pad to size of `struct sockaddr` */
    /* いわゆるパディング */
    unsigned char sin_zero[sizeof (struct sockaddr) -
                __SOCKADDR_COMMON_SIZE -
                sizeof (in_port_t) -
                sizeof (struct in_addr)
    ];
};
```

- `__SOCKADDR_COMMON` のマクロは `/usr/include/bits/sockaddr.h` に定義されている。

```c
// 2 Bytes
typedef unsigned short int sa_family_t;

#define __SOCKADDR_COMMON(sa_prefix)  \
  sa_family_t sa_prefix##family
```

- そこで、`sendto()` 関数の第五引数の `struct sockaddr` を調査してみる。
- まず、man で `sendto()` を調べると、`#include <sys/socket.h>` を定義する必要があるので、`/usr/include/sys/socket.h` を見に行く。そうすると、なんとなく `/usr/include/bits/socket.h` を見に行けば良い気がする。
- その結果、以下の定義が見つかる。

```c
struct sockaddr {
    __SOCKADDR_COMMON (sa_);
    char sa_data[14];
};
```

- `sendto` で `struct sockaddr_in` を `sockaddr` にキャストするのは、以下の理由が考えられる。
- また、これは、`struct sockaddr_in server` を `memset(&server, 0, sizeof(server));` で初期化するのと関連していそう。(推測)

```text
可能ですが、めんどくさいです。nomukenさんの回答のようポートとIPアドレスが入った変数からmemcpyするか、その値をビット演算でバイト単位に切り出してsa_data[0]～sa_data[5]にそれぞれ代入するなど。

struct sockaddr_inに値をセットするほうが簡単です。
面倒で読みにくい方法をとっても良いですが、普通は簡単で読みやすい手段を取ります。 
```

#### 参考
- [sockadrr構造体 sockadrr_in構造体について](https://teratail.com/questions/210977)

### recvfrom()
- man を確認する。

```bash
SYNOPSIS
       #include <sys/types.h>
       #include <sys/socket.h>

       ssize_t recv(int sockfd, void *buf, size_t len, int flags);

       ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                        struct sockaddr *src_addr, socklen_t *addrlen);

RETURN VALUE
       These calls return the number of bytes received, or -1 if an error occurred.  In the event of an error, errno is set to indicate the error.
```


### inet_addr()
- man を確認する。
- つまり、IP アドレスからネットワークバイトオーダのバイナリに変換する。適切に変換できない時は、`INADDR_NONE` が返る。

```c
SYNOPSIS
       #include <sys/socket.h>
       #include <netinet/in.h>
       #include <arpa/inet.h>

       in_addr_t inet_addr(const char *cp);

DESCRIPTION
       The inet_addr() function converts the Internet host address cp from IPv4 numbers-and-dots notation into binary data in network  byte  order.   If  the  input  is
       invalid,  INADDR_NONE  (usually -1) is returned.  Use of this function is problematic because -1 is a valid address (255.255.255.255).
```

- `/usr/include/netinet/in.h` に `typedef uint32_t in_addr_t` と定義されている。これは、前述で調査した `sockaddr_in.sin_addr.s_addr` のメンバに格納される。型も一致している。

### gethostbyname()
- man を確認する。

```bash
SYNOPSIS
       #include <netdb.h>
       extern int h_errno;

       struct hostent *gethostbyname(const char *name);

DESCRIPTION
       The  gethostbyname()  function  returns  a  structure of type hostent for the given host name.  Here name is either a hostname or an IPv4 address in standard dot
       notation (as for inet_addr(3)).  If name is an IPv4 address, no lookup is performed and gethostbyname() simply copies name into the h_name field and  its  struct
       in_addr  equivalent  into  the  h_addr_list[0] field of the returned hostent structure.  If name doesnt end in a dot and the environment variable HOSTALIASES is
       set, the alias file pointed to by HOSTALIASES will first be searched for name (see hostname(7) for the file format).  The current  domain  and  its  parents  are
       searched unless name ends in a dot.

   Historical
       The nsswitch.conf(5) file is the modern way of controlling the order of host lookups.

       In glibc 2.4 and earlier, the order keyword was used to control the order of host lookups as defined in /etc/host.conf (host.conf(5)).

       The hostent structure is defined in <netdb.h> as follows:

           struct hostent {
               char  *h_name;            /* official name of host */
               char **h_aliases;         /* alias list */
               int    h_addrtype;        /* host address type */
               int    h_length;          /* length of address */
               char **h_addr_list;       /* list of addresses */
           }
           // この下の行のマクロで `h_addr` が定義されている。
           #define h_addr h_addr_list[0] /* for backward compatibility */

       The members of the hostent structure are:

       h_name The official name of the host.

       h_aliases
              An array of alternative names for the host, terminated by a null pointer.

       h_addrtype
              The type of address; always AF_INET or AF_INET6 at present.

       h_length
              The length of the address in bytes.

       h_addr_list
              An array of pointers to network addresses for the host (in network byte order), terminated by a null pointer.

       h_addr The first address in h_addr_list for backward compatibility.

RETURN VALUE
       The  gethostbyname()  and  gethostbyaddr()  functions return the hostent structure or a null pointer if an error occurs.  On error, the h_errno variable holds an
       error number.  When non-NULL, the return value may point at static data, see the notes below.
```

- ちなみに、`/usr/include/netdb.h` を確認すると、`h_addr` は Ubuntu 18.04 上では、以下のように定義されている。
- したがって、Ubuntu 上ではこの変数を使うことができないので、直に `h_addr_list[0]` で呼び出すと、何も出力されなかった。疑問のまま一旦次に進む。

```c
#ifdef __USE_MISC
# define h_addr h_addr_list[0]
#endif
```

### getservbyname()
- man を確認する。

```bash
SYNOPSIS
       #include <netdb.h>

       struct servent *getservbyname(const char *name, const char *proto);

DESCRIPTION
       The getservbyname() function returns a servent structure for the entry from the database that matches the service name using protocol proto.  If proto  is  NULL,
       any protocol will be matched.  A connection is opened to the database if necessary.

       The servent structure is defined in <netdb.h> as follows:

           struct servent {
               char  *s_name;       /* official service name */
               char **s_aliases;    /* alias list */
               int    s_port;       /* port number */
               char  *s_proto;      /* protocol to use */
           }

       The members of the servent structure are:

       s_name The official name of the service.

       s_aliases
              A NULL-terminated list of alternative names for the service.

       s_port The port number for the service given in network byte order.

       s_proto
              The name of the protocol to use with this service.

RETURN VALUE
       The  getservent(),  getservbyname() and getservbyport() functions return a pointer to a statically allocated servent structure, or NULL if an error occurs or the
       end of the file is reached.
```

### ntohs()
- man を確認する。

```bash
SYNOPSIS
       #include <arpa/inet.h>

       uint16_t ntohs(uint16_t netshort);

DESCRIPTION
       The ntohs() function converts the unsigned short integer netshort from network byte order to host byte order.
```

## 3 章で作成する UDP Server のプログラムを実装する際に必要なシステムコール
- htonl()
- bind()

### bind()
- man を確認する。
- 以下の `bind()` の第二引数を確認すると、クライアント側の `sendto()` のように `sockaddr_in` 型のオブジェクトを作成する必要がある。

```bash
SYNOPSIS
       #include <sys/types.h>          /* See NOTES */
       #include <sys/socket.h>

       int bind(int sockfd, const struct sockaddr *addr,
                socklen_t addrlen);
```

### htonl(INADDR_ANY)
- man を確認する。
- Client 側のコードを書くときはお目にかからなかったコードであるが、これは、どのアドレスからリクエストが飛んできても処理をするという意味である。一方で、IP を指定すると、そのアドレス (から/へ) しか受け付けない設定である。

```bash
SYNOPSIS
       #include <arpa/inet.h>

       uint32_t htonl(uint32_t hostlong);

DESCRIPTION
       The htonl() function converts the unsigned integer hostlong from host byte order to network byte order.
```

## 3 章で作成する TCP Server のプログラムを実装する際に必要なシステムコール
- send
- recv
