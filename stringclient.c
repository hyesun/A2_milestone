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

void getinput(char* inputbuf)
{
    char text[20];
    fputs("enter some text: ", stdout);
    fflush(stdout);
    if (fgets(text, sizeof text, stdin) != NULL)
    {
        char *newline = strchr(text, '\n'); //search for newline character
        if (newline != NULL)
        {
            *newline = '\0'; //overwrite trailing newline
        }
        printf("text = \"%s\"\n", text);
    }
    inputbuf = &text;
}

char* padleft(char *string, int padded_len)
{
    char* pad = "0";
    int len = (int) strlen(string);
    if (len >= padded_len)
    {
        return string;
    }
    int i;
    for (i = 0; i < padded_len - len; i++) {
        strcat(string, pad);
    }
    return string;
}

char* itoa(int value, int base)
{
    int i = 0;
    int n = value/base;
    char* result = malloc(n+1);

    for(i=0; i<n+1 ;i++)
    {
        *(result+i) = "0123456789abcdef"[value % base];
    }

    *(result+i) = '\0';

    return result;
}

int main()
{
    //disable buffer for more interactive experience
    setbuf(stdout, NULL);

    printf("START\n");

    //get socket setup
    char server_address[MAXHOSTNAME + 1];
    int status=0;
    gethostname(server_address, MAXHOSTNAME);
    int socketfd=call_socket(server_address, PORTNUM);

    //get user input
    char * user_input = "hello world";
    int user_input_len = strlen(user_input);

    //massage
    char* hexchar = itoa(user_input_len, 16);    //convert integer to hex chars
    char* hexcharpad = padleft(hexchar, 4);  //pad with '0's
    printf("final result %s\n", hexcharpad);

    //pack it into message
    int msg_size = 4+2+user_input_len+1; //4 for 4 byte length, 2 for <, space>, 1 for null terminator
    char * msg = malloc(msg_size);
    printf("msg size is %i\n", msg_size);
    strcpy(msg, hexcharpad);
    *(msg+4)=',';
    *(msg+5)=' ';
    strcpy(msg+6, user_input);
    *(msg+msg_size-1)='\0';
    printf("msg is: %s\n", msg);

    //send something
    char* buf = "lolz";
    int len, bytes_sent;
    len = strlen(buf);
    bytes_sent = send(socketfd, buf, len, 0);

    //receiving stuff
    char* buf2 = malloc(sizeof(char[10]));
    status = recv(socketfd, buf2, 10, 0);

    printf("END\n");
    return 0;
}
