# 概要

- 疑問に思って調査したが、解決されなかったことのログを残す。

## send / sendto と recv / recvfrom の第一引数の sockfd について

- send / sendto と recv / recvfrom の第一引数の socketfd の値は同じでなければならないと思っていた。しかし、[scanport_udp.c](https://github.com/dilmnqvovpnmlib/NetworkProgramming/blob/main/kiso_tcp/source/linux/ch5/scanport_udp.c) によると、値が違うケースでも良い場合がある。これは、ユーザ空間で処理する受信データのフォーマットを変更したいからと推測している。。
- また、recvfrom の引数はパケットを受信する際のインターフェースになっている。そして、そのインターフェースに従って受信したデータをバッファに書き込む処理をしていると推測している。
