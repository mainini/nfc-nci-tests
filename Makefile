all:
	gcc -Wall -L /usr/local/lib/ -l nfc_nci_linux -o poll poll.c
	g++ -Wall -L /usr/local/lib/ -l pthread -l nfc_nci_linux -o poll_mt poll_mt.cpp

clean:
	rm poll
	rm poll_mt
