all: stringclient stringserver

stringclient: stringclient.o
	g++ stringclient.o -o stringclient

stringserver: stringserver.o
	g++ stringserver.o -o stringserver

stringclient.o: stringclient.c
	g++ -c stringclient.c

stringserver.o: stringserver.c 
	g++ -c stringserver.c

clean: 
	rm -rf *o stringclient stringserver
	