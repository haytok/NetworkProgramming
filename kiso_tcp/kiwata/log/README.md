# 概要
- 基礎からわかるTCP/IP ネットワーク実験プログラミング（第2版）を読んで、調査・検証したり、実装して得た知見をまとめていく。

## 知ったことや学んだこと

### プリプロセッサを使ってマルチプラットフォーム対応を行う
- `#ifdef __linux` を使用してコンパイル時にプラットフォームによって条件分岐を行う。

#### 検証プログラム
- `test_0.c`

#### 参考
- [C言語でLinux、Solaris向け処理を#ifdefで分岐](https://mokky14.hatenablog.com/entry/2013/05/29/113244)
- [第14回 ヘッダファイルとプリプロセッサ指令](https://dev.grapecity.co.jp/support/powernews/column/clang/014/page02.htm)
