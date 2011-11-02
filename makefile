all: stringclient stringserver

stringclient: stringclient.o
	gcc -pthread stringclient.o -o stringclient

stringserver: stringserver.o
	gcc -pthread stringserver.o -o stringserver

stringclient.o: stringclient.c
	gcc -c -pthread stringclient.c

stringserver.o: stringserver.c 
	gcc -c -pthread stringserver.c

clean: 
	rm -rf *o stringclient stringserver