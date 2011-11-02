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
#include <pthread.h>

#include "stringclient.h"

#define STRLENMAX 0xFFFF-1 //max input the user can enter
#define MAXHOSTNAME 100
#define PORTNUM 2000    //for testing purposes
#define BACKLOG 5

#define NUMTHREADS 2
#define READ_THREAD 0
#define SEND_THREAD 1
#define THREAD_DATA_SIZE 50

//create threads on stack
pthread_t threads[NUMTHREADS];
pthread_mutex_t mutexsum;

int thread_data_count;
typedef struct
{
	int send_ready; //send ready true means not ready to read
	char *mystring;
}thread_data;

thread_data thread_data_array[THREAD_DATA_SIZE];

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

char* getinput()
{
    char text[STRLENMAX];

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
    return (char*)(&text);
}

char* padleft(char *string, int padded_len)
{
    char* pad = "0";
    int len = (int) strlen(string);

    if (len >= padded_len)
    {
        //no need to do anything
        return string;
    }

    //we will return this
    char* newstring = (char*)malloc(padded_len+1);
    strcpy(newstring, pad); //need a basis first so use strcpy instead of strcat here

    int i;
    for (i = 1; i < padded_len - len; i++)
    {
        strcat(newstring, pad); //now we can use strcat
    }

    //finish up with our original string and terminator
    strcat(newstring,string);
    *(newstring+padded_len) = '\0';

    return newstring;
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
//thread for reading input
void * Read_Thread(void* arg)
{
	printf("Read Thread created\n");
	int num_bytes_read;
	int nbytes = 50;
	char *my_string;
	/* These 2 lines are the heart of the program. */
	int count;
	while(1)
	{
		my_string = (char *) malloc (nbytes + 1);
		num_bytes_read = getline (&my_string, &nbytes, stdin);\
		if (num_bytes_read == -1)
		  {
			puts ("ERROR!");
		  }
		else
		  {
			//mutex the following
			pthread_mutex_lock (&mutexsum);
			thread_data_array[thread_data_count].mystring = my_string;\
			thread_data_count++;
			pthread_mutex_unlock (&mutexsum);
		  }
	}
}

void * Send_Thread(void* arg)
{
	printf("Send Thread created\n");
	int socketfd = (int)(arg);
	while(1)
	{
		while(thread_data_count)
		{
			sleep(2); //sleep for 2 seconds
			send(socketfd, thread_data_array[0].mystring, // send first one on queue
					strlen(thread_data_array[0].mystring)+1, 0);

			//mutex lock
			pthread_mutex_lock (&mutexsum);
			thread_data_count--;
			//rearrange the queue
			int i;
			for(i=0; i< thread_data_count;i++)
			{
				thread_data_array[i] = thread_data_array[i+1];
			}
			pthread_mutex_unlock (&mutexsum);
			//mutex release
			char* stringfromserver = malloc(sizeof(char[300]));
			recv(socketfd, stringfromserver, 300, 0);
			printf("Server: %s", stringfromserver);
		}
	}

}

int main()
{
    //disable buffer for more interactive experience
    setbuf(stdout, NULL);

    //initialize mutex
    pthread_mutex_init(&mutexsum, NULL);
    thread_data_count = 0;

    printf("START\n");

    //get socket setup
    char server_address[MAXHOSTNAME + 1];
    int status=0;
    gethostname(server_address, MAXHOSTNAME);
    int socketfd=call_socket(server_address, PORTNUM);

    //thread for reading input
    if(pthread_create(&threads[READ_THREAD], NULL, Read_Thread, NULL))
    {
    	printf("ERROR creating thread %d", READ_THREAD);
    	exit(-1);
    }
    //thread for sending request
    if(pthread_create(&threads[SEND_THREAD], NULL, Send_Thread, (void*)socketfd))
    {
    	printf("ERROR creating thread %d", SEND_THREAD);
    	exit(-1);
    }
    /*
    //get user input
    char* input = getinput();
    int input_len = strlen(input) + 1; //1 for the null terminator

    //massage
    char* hexchar = itoa(input_len, 16);    //convert integer to hex chars
    char* hexcharpad = padleft(hexchar, 4);  //pad with '0's
    printf("final result %s\n", hexcharpad);

    //pack it into message
    int msg_size = 4+2+input_len; //4 for 4 byte length, 2 for <, space>
    char * msg = malloc(msg_size);
    printf("msg size is %i\n", msg_size);
    strcpy(msg, hexcharpad);
    *(msg+4)=',';
    *(msg+5)=' ';
    strcpy(msg+6, input);
    *(msg+msg_size-1)='\0';
    printf("msg is: [%s]\n", msg);

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
    */
    pthread_exit(NULL);
    pthread_mutex_destroy(&mutexsum);
}
