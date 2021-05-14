udps: udps.c
	gcc -o udps.out udps.c
	sudo ./udps.out

udpc: udpc.c
	gcc -o udpc.out udpc.c
	sudo ./udpc.out 192.168.11.16

tcps: tcps.c
	gcc -o tcps.out tcps.c
	sudo ./tcps.out

tcpc: tcpc.c
	gcc -o tcpc.out tcpc.c
	sudo ./tcpc.out 192.168.11.16

build: udps.c udpc.c tcps.c tcpc.c
	gcc -o udps.out udps.c && gcc -o udpc.out udpc.c && gcc -o tcps.out tcps.c && gcc -o tcpc.out tcpc.c

.PHONY: clean
clean:
	rm -rf *.out
