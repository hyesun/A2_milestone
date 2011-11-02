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

#define STRLENMAX 0xFFFF-1 //max input the user can enter
#define MAXHOSTNAME 100
#define PORTNUM 2000    //for testing purposes
#define BACKLOG 5

#define NUMTHREADS 2
#define READ_THREAD 0
#define SEND_THREAD 1
#define THREAD_DATA_SIZE 128

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

int call_socket(char *hostname, char *portnum)
{

	// connect to www.example.com port 80 (http)

	struct addrinfo hints, *res;
	int sockfd;

	// first, load up address structs with getaddrinfo():

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
	hints.ai_socktype = SOCK_STREAM;

	// we could put "80" instead on "http" on the next line:
	getaddrinfo(hostname, portnum, &hints, &res);

	// make a socket:

	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	// connect it to the address and port we passed in to getaddrinfo():

	connect(sockfd, res->ai_addr, res->ai_addrlen);

    return (sockfd);
}

//thread for reading input
void * Read_Thread(void* arg)
{
	//printf("Read Thread created\n");
	int num_bytes_read;
	int nbytes = 128;
	char *my_string;
	/* These 2 lines are the heart of the program. */
	int count;
	while(1)
	{
		my_string = (char *) malloc (nbytes + 1);
		num_bytes_read = getline (&my_string, &nbytes, stdin);
		if (num_bytes_read == -1)
		  {
			puts ("ERROR!");
		  }
		else
		  {
			//mutex the following
			pthread_mutex_lock (&mutexsum);
			thread_data_array[thread_data_count].mystring = my_string;
			thread_data_count++;
			pthread_mutex_unlock (&mutexsum);
		  }
	}
}

void * Send_Thread(void* arg)
{
	//printf("Send Thread created\n");
	int socketfd = (int)(arg);
	while(1)
	{
		while(thread_data_count)
		{
			//massage the input data
			char* input = thread_data_array[0].mystring;
			unsigned int input_len = strlen(input);  //this includes the null terminator
			unsigned int buffersize = 4+2+input_len; //4 for strlen, 2 for [, ]
            char* buffer = (char*)malloc(buffersize+1);

            strncpy(buffer, (char*)(&input_len), 4);    //4 byte
            *(buffer + 4) = ',';
            *(buffer + 5) = ' ';
            strcpy(buffer + 6, input);  //get the input string
            *(buffer+buffersize-1)='\0';    //just to be safe
            //massage complete

			send(socketfd, buffer, buffersize, 0);
			free(buffer);
			free(thread_data_array[0].mystring);

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
			char* stringfromserver = malloc(buffersize+1);
			recv(socketfd, stringfromserver, buffersize, 0);
			printf("Server: %s\n", stringfromserver+6);
			free(stringfromserver);
	        sleep(2); //sleep for 2 seconds
		}
	}

}

int main()
{
    //disable buffer for more interactive experience
    setbuf(stdout, NULL);

    //get env var
    char* server_address = getenv("SERVER_ADDRESS");
    char* server_port = getenv("SERVER_PORT");
    //int server_port_num = atoi(server_port);

    //initialize mutex
    pthread_mutex_init(&mutexsum, NULL);
    thread_data_count = 0;

    //get socket setup
    //gethostname(server_address, MAXHOSTNAME);
    int socketfd=call_socket(server_address, server_port);

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

    pthread_exit(NULL);
    pthread_mutex_destroy(&mutexsum);
}
