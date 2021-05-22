scanhost: scanhost.c
	gcc -o scanhost.out scanhost.c
	sudo ./scanhost.out 192.168.11.16 192.168.11.20

scanport_tcp: scanport_tcp.c
	gcc -o scanport_tcp.out scanport_tcp.c
	sudo ./scanport_tcp.out 192.168.11.16 0 100

scanport_udp: scanport_udp.c
	gcc -o scanport_udp.out scanport_udp.c
	sudo ./scanport_udp.out 192.168.11.16 0 100

build: scanhost.c scanport_tcp.c scanport_udp.c 
	gcc -o scanhost.out scanhost.c && gcc -o scanport_tcp.out scanport_tcp.c && gcc -o scanport_udp.out scanport_udp.c

.PHONY: clean
clean:
	rm -rf *.out
