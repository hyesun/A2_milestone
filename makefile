all: stringclient stringserver

stringclient: stringclient.o
	gcc stringclient.o -o stringclient

stringserver: stringserver.o
	gcc stringserver.o -o stringserver

stringclient.o: stringclient.c
	gcc -c stringclient.c

stringserver.o: stringserver.c 
	gcc -c stringserver.c

clean: 
	rm -rf *o stringclient stringserver
	