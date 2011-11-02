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

#include "stringserver.h"

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
    //disable buffer for more interactive experience
    setbuf(stdout, NULL);

    //char a[]= " red blue g awek dd ";
    //titlecaps(a);
    //printf("%s", a);

    int status1 = 1;
    int status2 = 1;

    int socketfd = establish(PORTNUM);
    printf("socketfd = %i", socketfd);

    int newsocketfd = get_connection(socketfd);
    printf("newsocketfd = %i", newsocketfd);

    while(1 && status1 && status2)
    {
        char* buf = malloc(sizeof(char[300]));
		status1 = recv(newsocketfd, buf, 300, 0);
	    printf("\nnum of bytes read: %i\n", status1);
	    printf("read: %s", buf);
	    titlecaps(buf);
	    status2 = send(newsocketfd, buf, 300, 0);
	    free(buf);
    }
    /*
    //receiving stuff
    //int recv(int sockfd, void *buf, int len, int flags);
    char* buf = malloc(sizeof(char[10]));

    status = recv(newsocketfd, buf, 10, 0);

    printf("num of bytes read: %i", status);
    printf("read: %s", buf);


    printf("END\n");
    */
    return 0;
}
