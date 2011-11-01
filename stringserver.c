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

#include "stringserver.h"

#define MAXHOSTNAME 100
#define PORTNUM 2000    //for testing purposes
#define BACKLOG 5

int establish(unsigned short portnum)
{
    char myname[MAXHOSTNAME + 1];
    int s;
    struct sockaddr_in sa;
    struct hostent *hp;
    memset(&sa, 0, sizeof(struct sockaddr_in)); /* clear our address */

    gethostname(myname, MAXHOSTNAME); /* who are we? */
    hp = gethostbyname(myname); /* get our address info */
    if (hp == NULL) /* we don't exist !? */
        return (-1);

    sa.sin_family = hp->h_addrtype; /* this is our host address */
    sa.sin_port = htons(portnum); /* this is our port number */

    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) /* create socket */
        return (-1);
    if (bind(s, (struct sockaddr*)&sa, sizeof(struct sockaddr_in)) < 0) {
        close(s);
        return (-1); /* bind address to socket */
    }
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

int main()
{
    printf("START\n");

    int status = 0;

    int socketfd = establish(2000);
    status = get_connection(socketfd);

    printf("END\n");
    return 0;
}
