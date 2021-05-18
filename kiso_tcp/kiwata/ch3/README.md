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
- setsockopt()
- gethostbyaddr()
- getservbyport()

### select()

- man を確認する。
- 第二引数の readfds には、recv や recvfrom による受信を検査する fd を指定する。

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

EXAMPLE
       #include <stdio.h>
       #include <stdlib.h>
       #include <sys/time.h>
       #include <sys/types.h>
       #include <unistd.h>

       int
       main(void)
       {
           fd_set rfds;
           struct timeval tv;
           int retval;

           /* Watch stdin (fd 0) to see when it has input. */

           FD_ZERO(&rfds);
           FD_SET(0, &rfds);

           /* Wait up to five seconds. */

           tv.tv_sec = 5;
           tv.tv_usec = 0;

           retval = select(1, &rfds, NULL, NULL, &tv);
           /* Don't rely on the value of tv now! */

           if (retval == -1)
               perror("select()");
           else if (retval)
               printf("Data is available now.\n");
               /* FD_ISSET(0, &rfds) will be true. */
           else
               printf("No data within five seconds.\n");

           exit(EXIT_SUCCESS);
       }
```

- `fd_set` は、ファイルディスクリプタの集合を表す。

- `FD_ZERO(&fds)` は、初期化 (集合を空にする) するヘルパーマクロである。

- `FD_SET(fd, &fds)` は、fd を集合に加える。

- `FD_CLR(fd, &fds)` は、fd を集合から取り除く。

- `FD_ISSET(fd, &fds)` は、fd がその集合に含まれているかを確認する。

- `#define FD_SETSIZE 1024` のマクロがどこかで定義されている。

#### 参考

- [SELECT](https://linuxjm.osdn.jp/html/LDP_man-pages/man2/select.2.html)

- [\[C言語\] ライブラリの関数などメモ](https://qiita.com/edo_m18/items/7414028fd91269e5427d#select)

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
- 少し別の言い方をすると、この関数は、人間にとって可読性のある 192.168.11.6 などの IP アドレスをプロトコルスタックで必要なネットワークバイトオーダのバイナリに変換するためのライブラリ関数である。

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

## 3 章で作成する TCP Client のプログラムを実装する際に必要なシステムコール

## 3 章で作成する TCP Server のプログラムを実装する際に必要なシステムコール

- listen()
- accept()
- send()
- recv()

### listen()

- man を確認する。
- 第二引数の `backlog` は、接続保留中のキューの最大長を定義する。このパラメータで設定した値より大きい数の接続要求が来ると、クライアント側は `ECONNREFUSED` を受け取る。つまり、`accept` されるのを待っているソケットの数のことを表しているとも言える。

```bash
SYNOPSIS
       #include <sys/types.h>          /* See NOTES */
       #include <sys/socket.h>

       int listen(int sockfd, int backlog);

DESCRIPTION
       listen()  marks  the  socket  referred  to  by  sockfd  as  a passive socket, that is, as a socket that will be used to accept incoming connection requests using
       accept(2).

       The sockfd argument is a file descriptor that refers to a socket of type SOCK_STREAM or SOCK_SEQPACKET.

       The backlog argument defines the maximum length to which the queue of pending connections for sockfd may grow.  If a connection request arrives when the queue is
       full,  the  client  may receive an error with an indication of ECONNREFUSED or, if the underlying protocol supports retransmission, the request may be ignored so
       that a later reattempt at connection succeeds.

RETURN VALUE
       On success, zero is returned.  On error, -1 is returned, and errno is set appropriately.

NOTES
       To accept connections, the following steps are performed:

           1.  A socket is created with socket(2).

           2.  The socket is bound to a local address using bind(2), so that other sockets may be connect(2)ed to it.

           3.  A willingness to accept incoming connections and a queue limit for incoming connections are specified with listen().

           4.  Connections are accepted with accept(2).

       POSIX.1 does not require the inclusion of <sys/types.h>, and this header file is not required on Linux.  However, some historical (BSD) implementations  required
       this header file, and portable applications are probably wise to include it.

       The  behavior  of the backlog argument on TCP sockets changed with Linux 2.2.  Now it specifies the queue length for completely established sockets waiting to be
       accepted,  instead  of  the  number  of  incomplete  connection  requests.   The  maximum  length  of  the  queue  for  incomplete  sockets  can  be  set   using
       /proc/sys/net/ipv4/tcp_max_syn_backlog.   When syncookies are enabled there is no logical maximum length and this setting is ignored.  See tcp(7) for more infor‐
       mation.

       If the backlog argument is greater than the value in /proc/sys/net/core/somaxconn, then it is silently truncated to that value; the default value in this file is
       128.  In kernels before 2.4.25, this limit was a hard coded value, SOMAXCONN, with the value 128.
```

### accept()

- man を確認する。

```bash
SYNOPSIS
       #include <sys/types.h>          /* See NOTES */
       #include <sys/socket.h>

       int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

DESCRIPTION
       The  accept()  system  call  is  used with connection-based socket types (SOCK_STREAM, SOCK_SEQPACKET).  It extracts the first connection request on the queue of pending connections for the listening socket, sockfd, creates a new connected socket, and returns a new file descriptor referring to that socket.  The newly created socket is not in the listening state.  The original socket sockfd is unaffected by this call.

RETURN VALUE
       On success, these system calls return a nonnegative integer that is a file descriptor for the accepted socket.  On error, -1 is returned, and errno is set appropriately.
```

### recv()

- man を確認する。

```bash
SYNOPSIS
       #include <sys/types.h>
       #include <sys/socket.h>

       ssize_t recv(int sockfd, void *buf, size_t len, int flags);

DESCRIPTION
       The  recv(), recvfrom(), and recvmsg() calls are used to receive messages from a socket.  They may be used to receive data on both connectionless and connection-
       oriented sockets.  This page first describes common features of all three system calls, and then describes the differences between the calls.

RETURN VALUE
       These calls return the number of bytes received, or -1 if an error occurred.  In the event of an error, errno is set to indicate the error.
```

### send()

- まず、いつも通りに man を確認する。

```bash
SYNOPSIS
       #include <sys/types.h>
       #include <sys/socket.h>

       ssize_t send(int sockfd, const void *buf, size_t len, int flags);

DESCRIPTION
       The system calls send(), sendto(), and sendmsg() are used to transmit a message to another socket.

       The  send()  call  may  be  used  only when the socket is in a connected state (so that the intended recipient is known).  The only difference between send() and
       write(2) is the presence of flags.  With a zero flags argument, send() is equivalent to write(2).  Also, the following call

           send(sockfd, buf, len, flags);

       is equivalent to

           sendto(sockfd, buf, len, flags, NULL, 0);

       The argument sockfd is the file descriptor of the sending socket.

       If sendto() is used on a connection-mode (SOCK_STREAM, SOCK_SEQPACKET) socket, the arguments dest_addr and addrlen are ignored (and  the  error  EISCONN  may  be
       returned  when  they are not NULL and 0), and the error ENOTCONN is returned when the socket was not actually connected.  Otherwise, the address of the target is
       given by dest_addr with addrlen specifying its size.  For sendmsg(), the address of the target is given by  msg.msg_name,  with  msg.msg_namelen  specifying  its
       size.

       For send() and sendto(), the message is found in buf and has length len.  For sendmsg(), the message is pointed to by the elements of the array msg.msg_iov.  The
       sendmsg() call also allows sending ancillary data (also known as control information).

       If the message is too long to pass atomically through the underlying protocol, the error EMSGSIZE is returned, and the message is not transmitted.

       No indication of failure to deliver is implicit in a send().  Locally detected errors are indicated by a return value of -1.

       When the message does not fit into the send buffer of the socket, send() normally blocks, unless the socket has been placed in nonblocking  I/O  mode.   In  non‐
       blocking  mode  it  would  fail  with the error EAGAIN or EWOULDBLOCK in this case.  The select(2) call may be used to determine when it is possible to send more
       data.

RETURN VALUE
       On success, these calls return the number of bytes sent.  On error, -1 is returned, and errno is set appropriately.
```

### listen() の第二引数の backlog パラメータについての検証

- backlog パラメータはカーネルパラメータとして決まっていて、デフォルトでは 128 が設定されている。
- ss コマンドを使用して backlog パラメータを変更した時に同時接続できるクライアント数がどれくらいになるのかを Python コードで検証している興味深い記事である。

#### 参考

- [Linuxのbacklogについて調べてみる](https://kazuhira-r.hatenablog.com/entry/2019/07/10/015733)

- [ssコマンドのRecv-Qが、backlog＋1の値まで上がるのはどうして？](https://kazuhira-r.hatenablog.com/entry/2019/07/27/200040)

- [Socket()とかBind()とかを理解する](https://qiita.com/Michinosuke/items/0778a5344bdf81488114)

- [コネクション型通信：サーバプログラムの作成](http://research.nii.ac.jp/~ichiro/syspro98/server.html)

### setsockopt()

- man を確認する。
- サーバーで同じポート番号で bind を繰り返すとエラーが出る。`setsockopt` を使うと、このエラーをエスケープすることができる。
- また、書籍では、TCP のウィンドウサイズを変更する際にも使用できると記述があった。

```bash
SYNOPSIS
       #include <sys/types.h>          /* See NOTES */
       #include <sys/socket.h>

       int getsockopt(int sockfd, int level, int optname,
                      void *optval, socklen_t *optlen);
       int setsockopt(int sockfd, int level, int optname,
                      const void *optval, socklen_t optlen);

DESCRIPTION
       getsockopt()  and  setsockopt() manipulate options for the socket referred to by the file descriptor sockfd.  Options may exist at multiple protocol levels; they
       are always present at the uppermost socket level.

       When manipulating socket options, the level at which the option resides and the name of the option must be specified.  To manipulate options at the  sockets  API
       level, level is specified as SOL_SOCKET.  To manipulate options at any other level the protocol number of the appropriate protocol controlling the option is sup‐
       plied.  For example, to indicate that an option is to be interpreted by the TCP protocol, level should be set to the protocol number of TCP; see getprotoent(3).

       The arguments optval and optlen are used to access option values for setsockopt().  For getsockopt() they identify a buffer in which the value for the  requested
       option(s)  are  to be returned.  For getsockopt(), optlen is a value-result argument, initially containing the size of the buffer pointed to by optval, and modi‐
       fied on return to indicate the actual size of the value returned.  If no option value is to be supplied or returned, optval may be NULL.

       Optname and any specified options are passed uninterpreted to the appropriate protocol module for interpretation.  The include file <sys/socket.h> contains defi‐
       nitions for socket level options, described below.  Options at other protocol levels vary in format and name; consult the appropriate entries in section 4 of the
       manual.

       Most socket-level options utilize an int argument for optval.  For setsockopt(), the argument should be nonzero to enable a boolean option, or zero if the option
       is to be disabled.

       For a description of the available socket options see socket(7) and the appropriate protocol man pages.

RETURN VALUE
       On success, zero is returned for the standard options.  On error, -1 is returned, and errno is set appropriately.
```

### 参考

- [車輪のx発明 ~B.G's Blog~](https://bg1.hatenablog.com/entry/2015/08/19/210000)

### gethostbyaddr()

- man を確認する。

```bash
SYNOPSIS
       #include <netdb.h>
       extern int h_errno;

       #include <sys/socket.h>       /* for AF_INET */
       struct hostent *gethostbyaddr(const void *addr,
                                     socklen_t len, int type);

DESCRIPTION
       The gethostbyname*(), gethostbyaddr*(), herror(), and hstrerror() functions are obsolete.  Applications should use getaddrinfo(3), getnameinfo(3),  and  gai_str‐
       error(3) instead.

       The  gethostbyaddr()  function  returns a structure of type hostent for the given host address addr of length len and address type type.  Valid address types are
       AF_INET and AF_INET6.  The host address argument is a pointer to a struct of a type depending on the address type, for  example  a  struct  in_addr  *  (probably
       obtained via a call to inet_addr(3)) for address type AF_INET.

RETURN VALUE
       The  gethostbyname()  and  gethostbyaddr()  functions return the hostent structure or a null pointer if an error occurs.  On error, the h_errno variable holds an
       error number.  When non-NULL, the return value may point at static data, see the notes below.
```

### getservbyport()

- man を確認する。

```bash
SYNOPSIS
       #include <netdb.h>

       struct servent *getservbyport(int port, const char *proto);

DESCRIPTION
       The  getservbyport() function returns a servent structure for the entry from the database that matches the port port (given in network byte order) using protocol
       proto.  If proto is NULL, any protocol will be matched.  A connection is opened to the database if necessary.

RETURN VALUE
       The  getservent(),  getservbyname() and getservbyport() functions return a pointer to a statically allocated servent structure, or NULL if an error occurs or the
       end of the file is reached.

FILES
       /etc/services
              services database file
```

## クライアント側で bind() を行うかどうかについて

- 書籍 p.101, p.108 に記述がある。
- UCP, TCP で通信をする際に、クライアント側では bind() を省略するのが一般的である。
- クライアント側のポート番号は、最初に sendto() / send() した段階で OS が自動的に割り当てる。ただし、IP アドレスは固定されない。
- NIC が複数ある時や IP アドレスが複数紐付けられている時は、ネットワークの状態などにより送信する始点 IP アドレスが変化する場合がある。
- 変化させたくない時には、クライアントでも sendto() / send() する前に bind() を使用して、パケットの送受信に使用する IP アドレスやポート番号を指定することができる。

## bind() のマニュアルに Example がある

```c
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MY_SOCK_PATH "/somepath"
#define LISTEN_BACKLOG 50

#define handle_error(msg) \
       do { perror(msg); exit(EXIT_FAILURE); } while (0)

int
main(int argc, char *argv[])
{
       int sfd, cfd;
       struct sockaddr_un my_addr, peer_addr;
       socklen_t peer_addr_size;

       sfd = socket(AF_UNIX, SOCK_STREAM, 0);
       if (sfd == -1)
              handle_error("socket");

       memset(&my_addr, 0, sizeof(struct sockaddr_un));
                            /* Clear structure */
       my_addr.sun_family = AF_UNIX;
       strncpy(my_addr.sun_path, MY_SOCK_PATH,
              sizeof(my_addr.sun_path) - 1);

       if (bind(sfd, (struct sockaddr *) &my_addr,
              sizeof(struct sockaddr_un)) == -1)
              handle_error("bind");

       if (listen(sfd, LISTEN_BACKLOG) == -1)
              handle_error("listen");

       /* Now we can accept incoming connections one
       at a time using accept(2) */

       peer_addr_size = sizeof(struct sockaddr_un);
       cfd = accept(sfd, (struct sockaddr *) &peer_addr,
                     &peer_addr_size);
       if (cfd == -1)
              handle_error("accept");

       /* Code to deal with incoming connection(s)... */

       /* When no longer required, the socket pathname, MY_SOCK_PATH
       should be deleted using unlink(2) or remove(3) */
}
```

## UDP Client と UDP Server をインクリメンタルに実装していく

- [ ] UDP Client が UDP Server に固定のメッセージを送り、サーバ側で送信されたメッセージを出力する。
  - UDP Client の実装で必要なこと
    - socket()
    - sendto()
      - struct sockaddr_in オブジェクトを作成する。
      - inet_addr() (ネットワークアドレスをネットワークバイトオーダに変換する)
      - htons()
    - recvfrom()
      - サーバ側からメッセージが飛んでくるまで、それ以降の処理がブロックされる ???
  - UDP Server の実装で必要なこと
    - socket()
    - bind()
      - bind するサーバの情報を作成して socket オブジェクトに紐付ける。
    - recvfrom()
- [ ] UDP Server が UDP Client からのメッセージを受け取ると返答を返し、その返答を UDP Client で表示する。
  - UDP Client の実装で必要なこと
    - 特になし
  - UDP Server の実装で必要なこと
    - sendto()
- [ ] UDP Client 側で、固定のメッセージを送信するのではなく。標準入力から受け取った値を送信するように修正する。
  - select() を使って標準入力とソケットの受信を選択するようにする。

### 実装途中でエラーが出た

- Bad Address のエラーが生じた。`recvfrom()` の第二引数と第三引数の指定が悪かった。本通りに以下のように予め指定したサイズの領域を確保する必要があった。

```c
#define BUFSIZE 
char recv_buf[BUFSIZE];
recvfrom(s, recv_buf, )
```

### コミットの過程

- Phase 1
  - [bdea34def25204c5b86df90097f3549e3ce4ef94](https://github.com/dilmnqvovpnmlib/NetworkProgramming/commit/bdea34def25204c5b86df90097f3549e3ce4ef94)

- Phase 2
  - [6e1b32cd5bc4fde595820a5a0c74ef0104e00bf6](https://github.com/dilmnqvovpnmlib/NetworkProgramming/commit/6e1b32cd5bc4fde595820a5a0c74ef0104e00bf6)

- Phase 3
  - [1d66c652fea7352c7cbd934cfaff217b2e298b78](https://github.com/dilmnqvovpnmlib/NetworkProgramming/commit/1d66c652fea7352c7cbd934cfaff217b2e298b78)

## TCP Client と TCP Server をインクリメンタルに実装していく

- [ ] TCP Server でコネクションを張る準備
  - socket()
    - ソケットを開く。
  - bind()
    - ポート番号やアドレスなどのサーバの情報を紐付ける。
  - listen()
    - コネクションの受付を開始する。
  - accept()
    - コネクションを受け付ける。
- [ ] TCP Client でコネクションを確立する準備
  - socket()
    - ソケットを開く。
  - connect()
    - コネクションを確立する。
- [ ] TCP Client からコマンドを送信
  - 送信メッセージを作成する。
  - send()
- [ ] TCP Server でコマンドを受信し、返答
  - recv()
  - 受信メッセージを解析する。
  - send()
- [ ] TCP Client からコネクションを閉じるコマンドの送信
  - 送信メッセージを作成する。
  - send()
  - close()
- [ ] TCP Server でコネクションを閉じる
  - recv()
  - 受信メッセージを解析する。
  - close()

## man 7

- 定義されているマクロ (ex. INADDR_ANY) を調べることができる。(できるだけ Web で検索をかけずに実装してきたい。時間的な観点からも生産性が上がり、実装する際の手順を画一的にすることができる。)

- ex) man 7 ip

### 疑問

- socket() を呼び出して、エラーが生じた場合、ソケットを close() する必要があるんかと思った。
- man で socket() の返り値を確認してみると、以下のような記述がされていた。
- 成功すれば新しくソケットの fd を返し、失敗すれば -1 を返すと解釈できる。つまり、そもそもソケットの fd が返ってきてないから、close() のしようが無い認識で合ってる？？？

```bash
RETURN VALUE
       On success, a file descriptor for the new socket is returned.  On error, -1 is returned, and errno is set appropriately.
```

## connect() を呼び出した後に send() をしないと処理ばブロックされる

- main 関数が以下の状態だと、while 文にまで処理が進まない。

```c
int main(int argc, char **argv) {
    int s;
    struct sockaddr_in server;
    char *address;
    in_addr_t dest_ip;
    int port;
    char send_buf[BUF_SIZE];

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        die("socket");
    }

    address = DEFAULT_DEST_ADDRESS;
    dest_ip = inet_addr(address);
    port = DEFAULT_PORT;

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(dest_ip);
    server.sin_port = htons(port);
    if (connect(s, (struct sockaddr *) &server, sizeof(server)) < 0) {
        die("connect");
    }

    while (true) {
        puts("hello");
    }

    close(s);

    return 0;
}
```

## プログラムを変更した時の手順
- TCP Client のプログラムを停止させる。
- TCP Server のプログラムを停止させる。
- この順でプログラムを止めないと、`bind: Address alreay in use` のエラーが生じてしまう。
