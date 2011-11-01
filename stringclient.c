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
    sa.sin_addr.s_addr = INADDR_ANY; /*use a specific IP of host*/

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
    //disable buffer for more interactive experience
    setbuf(stdout, NULL);

    printf("START\n");

    char myname[MAXHOSTNAME + 1];
    int status=0;

    gethostname(myname, MAXHOSTNAME);
    int socketfd=call_socket(myname, 2000);
    printf("socketfd = %i", socketfd);

    //send something
    char* buf = "lolz";
    int len, bytes_sent;
    len = strlen(buf);
    printf("length = %i", len);
    bytes_sent = send(socketfd, buf, len, 0);
    printf("bytes sent = %i", bytes_sent);


    //receiving stuff
    //int recv(int sockfd, void *buf, int len, int flags);
    char* buf2 = malloc(sizeof(char[10]));

    status = recv(socketfd, buf2, 10, 0);

    printf("num of bytes read: %i", status);
    printf("read: %s", buf2);

    printf("END\n");
    return 0;
}
