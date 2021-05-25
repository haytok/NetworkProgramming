arpupdate_reply: arpupdate.c
	gcc -o arpupdate.out arpupdate.c
	sudo ./arpupdate.out wlp2s0 192.168.11.4 00:01:02:03:04:05 reply

arpupdate_request: arpupdate.c
	gcc -o arpupdate.out arpupdate.c
	sudo ./arpupdate.out wlp2s0 192.168.11.4 00:01:02:03:04:05 request

arpupdate_reply_and_request: arpupdate.c
	gcc -o arpupdate.out arpupdate.c
	sudo ./arpupdate.out wlp2s0 192.168.11.4 00:01:02:03:04:05

build: arpupdate.c
	gcc -o arpupdate.out arpupdate.c

.PHONY: clean
clean:
	rm -rf *.out
