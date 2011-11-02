#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>

#define MAXHOSTNAME 100
#define PORTNUM 0    //for testing purposes
#define BACKLOG 5

int establish(unsigned short portnum)
{
    char server_address[MAXHOSTNAME + 1];
    int s;
    struct addrinfo hints, *res;
    struct hostent *hp;

    // modern way of doing things with getaddrinfo()

    int sockfd;

    // first, load up address structs with getaddrinfo():

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    getaddrinfo(NULL, "0", &hints, &res);

    // make a socket:
    // (you should actually walk the "res" linked list and error-check!)

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    // bind it to the port we passed in to getaddrinfo():

    bind(sockfd, res->ai_addr, res->ai_addrlen);


    listen(s, 5); /* max # of queued connects */

    //read the allocated port number
    int length = sizeof(hints);
    getsockname(s, (struct sockaddr*)&hints, &length);

    //print out the required env var
    printf("SERVER_ADDRESS %s\n", server_address);
    printf("SERVER_PORT %i\n", ntohs(((struct sockaddr_in*)(hints.ai_addr))->sin_port));

    return (s);
}

int get_connection(int s)
{
    int t; /* socket of connection */
    if ((t = accept(s, NULL, NULL)) < 0) /* accept connection if there is one */
        return (-1);
    return (t);
}

void titlecaps(char* text)
{
    int i;
    int j = strlen(text);

    for (i = 0; i < j; i++) //traverse through string
    {
        if (i==0)
            *(text+i) = toupper(*(text+i));
        else if (*(text+i-1) == ' ')
            *(text+i) = toupper(*(text+i));
    }
}

int main()
{
    //copied
    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number

    int listener;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;

    char buf[256];    // buffer for client data
    int nbytes;

    char remoteIP[INET6_ADDRSTRLEN];

    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, j, rv;

    struct addrinfo hints, *ai, *p;

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);
    //end copied


    //disable buffer for more interactive experience
    setbuf(stdout, NULL);

    int status1 = 1;
    int status2 = 1;

    listener = establish(PORTNUM);
    if(listener == -1)
    	exit(-1);

    //not needed?
    /*
    int newsocketfd = get_connection(socketfd);
    if(newsocketfd == -1)
    	exit(-1);
    */

    //copied
    // add the listener to the master set
    FD_SET(listener, &master);

    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one

    int counter=0;
    // main loop
    for(;;)
    {
        counter++;
        //printf("\n-----------------main loop: %i--------------\n", counter);
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1)
        {
            perror("select");
            exit(4);
        }

        // run through the existing connections looking for data to read
        for(i = 0; i <= fdmax; i++)
        {
            //printf("looping thru existing connections. fdmax=%i, i=%i\n", fdmax, i);

            if (FD_ISSET(i, &read_fds))
            {
                // we got one!!
                //printf("we got one!\n");
                if (i == listener)
                {
                    //printf("i is listener\n");
                    newfd = accept(listener,NULL, NULL);

                    if (newfd == -1)
                    {
                        perror("accept");
                    }
                    else
                    {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax)
                        {    // keep track of the max
                            fdmax = newfd;
                        }
                    }
                }
                else
                {
                    //printf("i is not listener. do routine\n");
                    //prepare buffer and read stuff into it
                    char* buffer = (char*)malloc(100);
                    status1 = recv(i, buffer, 100, 0);
                    printf("%i", *(int*)buffer);
                    printf("%s\n", buffer+4);

                    if(status1 <= 0 )
                    {
                    	close(i); // bye!
                    	FD_CLR(i, &master); // remove from master set
                    }

                    //string manipulation
                    titlecaps((buffer+6));
                    status2 = send(i, buffer, status1, 0);

                    //printf("sent!\n");

                    //cleanup
                    free(buffer);
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } // END for(;;)--and you thought it would never end!
    return 0;
}
