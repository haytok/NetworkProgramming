tcpsyn: tcpsyn.c
	gcc -o tcpsyn.out tcpsyn.c
	sudo ./tcpsyn.out 127.0.0.1 8000 192.168.11.16 5000 1

tcprst: tcprst.c
	gcc -o tcprst.out tcprst.c
	sudo ./tcprst.out 127.0.0.1 8000 192.168.11.16 5000 111111111 222222222

tcpjack: tcpjack.c
	gcc -o tcpjack.out tcpjack.c
	sudo ./tcpjack.out 127.0.0.1 8000 192.168.11.16 5000 111111111 222222222

build: tcpsyn.c tcprst.c tcpjack.c
	gcc -o tcpsyn.out tcpsyn.c && gcc -o tcprst.out tcprst.c && gcc -o tcpjack.out tcpjack.c

.PHONY: clean
clean:
	rm -rf *.out
