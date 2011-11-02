all: stringClient stringServer

stringClient: stringClient.o
	gcc -pthread stringClient.o -o stringClient

stringServer: stringServer.o
	gcc -pthread stringServer.o -o stringServer

stringClient.o: stringClient.c
	gcc -c -pthread stringClient.c

stringServer.o: stringServer.c 
	gcc -c -pthread stringServer.c

clean: 
	rm -rf *o stringClient stringServer
