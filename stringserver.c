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
#define PORTNUM 2000    //for testing purposes
#define BACKLOG 5

int establish(unsigned short portnum)
{
    char server_address[MAXHOSTNAME + 1];
    int s;
    struct sockaddr_in sa;
    struct hostent *hp;
    memset(&sa, 0, sizeof(struct sockaddr_in)); /* clear our address */

    gethostname(server_address, MAXHOSTNAME); /* who are we? */
    hp = gethostbyname(server_address); /* get our address info */
    if (hp == NULL) /* we don't exist !? */
        return (-1);

    sa.sin_family = hp->h_addrtype; /* this is our host address */
    sa.sin_port = htons(portnum); /* this is our port number */
    sa.sin_addr.s_addr = INADDR_ANY; /*use a specific IP of host*/

    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) /* create socket */
        return (-1);
    if (bind(s, (struct sockaddr*)&sa, sizeof(struct sockaddr_in)) < 0)
    {
        //error
        close(s);
        return (-1);
    }

    //this is dynamically allocated port number
    int server_port = (int)sa.sin_port;

    //print out requred env var
    printf("SERVER_ADDRESS %s\n", server_address);
    printf("SERVER_PORT %i\n", server_port);

    listen(s, 3); /* max # of queued connects */
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

    // main loop
    for(;;)
    {
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1)
        {
            perror("select");
            exit(4);
        }

        // run through the existing connections looking for data to read
        for(i = 0; i <= fdmax; i++)
        {
            if (FD_ISSET(i, &read_fds))
            { // we got one!!
                if (i == listener)
                {
                    // handle new connections
                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener,
                        (struct sockaddr *)&remoteaddr,
                        &addrlen);

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
                    //prepare buffer and read stuff into it
                    char* buffer = (char*)malloc(100);
                    status1 = recv(newfd, buffer, 100, 0);

                    //checking
                    int strlen = *(int*)buffer;

                    //string manipulation
                    titlecaps((buffer+6));
                    status2 = send(newfd, buffer+6, strlen, 0);

                    //cleanup
                    free(buffer);
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } // END for(;;)--and you thought it would never end!
    return 0;
}
