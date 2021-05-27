redirect: redirect.c
	gcc -o redirect.out redirect.c
	sudo ./redirect.out 192.168.11.4 192.168.11.1 192.168.11.99 192.168.3.104

scanroute: scanroute.c
	gcc -o scanroute.out scanroute.c
	sudo ./scanroute.out 8.8.8.8

build: redirect.c scanroute.c
	gcc -o redirect.out redirect.c && gcc -o scanroute.out scanroute.c

.PHONY: clean
clean:
	rm -rf *.out
