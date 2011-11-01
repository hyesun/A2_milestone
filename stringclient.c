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

#include "stringclient.h"

#define MAXHOSTNAME 100
#define PORTNUM 2000    //for testing purposes
#define BACKLOG 5

int call_socket(char *hostname, unsigned short portnum)
{
    struct sockaddr_in sa;
    struct hostent *hp;
    int a, s;
    if ((hp = gethostbyname(hostname)) == NULL)
    {
        errno = ECONNREFUSED; /* do we know the host's address*/
        return (-1); /* no */
    }

    memset(&sa, 0, sizeof(sa));
    memcpy((char *) &sa.sin_addr, hp->h_addr, hp->h_length); /* set address */
    sa.sin_family = hp->h_addrtype;
    sa.sin_port = htons((u_short) portnum);

    if ((s = socket(hp->h_addrtype, SOCK_STREAM, 0)) < 0) /* get socket */
        return (-1);

    if (connect(s, (struct sockaddr*)&sa, sizeof sa) < 0) { /* connect */
        close(s);
        return (-1);
    }

    return (s);
}

int main()
{
    //get user input
    printf("START\n");

    char myname[MAXHOSTNAME + 1];
    int status=0;

    gethostname(myname, MAXHOSTNAME);
    int socketfd=call_socket(myname, 2000);

    printf("END\n");
    return 0;
}
