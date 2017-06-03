#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/types.h>      /* sockets */
#include <sys/socket.h>      /* sockets */
#include <netinet/in.h>      /* internet sockets */
#include <netdb.h>          /* gethostbyaddr */
#include <unistd.h>          /* fork */  
#include <stdlib.h>          /* exit */
#include <ctype.h>          /* toupper */


#include "definitions_contentserver.h"
//#include "status.h"

pthread_mutex_t mtx_client;

void * thread_client_function(void *arg) 
{
	int DELAY, bytesRead;
	int *myArg = (int *)arg;
	char *split, *UNIQUE_ID;
	char messageFromClient[buf_SIZE];

	memset(messageFromClient, 0, buf_SIZE);

	pthread_mutex_lock(&mtx_client);	//LOCK

	if ((bytesRead = read( (*myArg), messageFromClient, buf_SIZE)) > 0 )
    {
    	pthread_mutex_unlock(&mtx_client);	//UNLOCK

    	printf("(content_server) message from mirror manager received: '%s'\n", messageFromClient);

    	char *tempptr = NULL;

        split = strtok_r(messageFromClient, " ", &tempptr);

        // check if messageFromClient is "LIST"
	    if(strcmp(split, "LIST") == 0)
	    {
	    	pthread_mutex_lock(&mtx_client);	//LOCK

	        write( (*myArg), "got your message mate", buf_SIZE );

	        pthread_mutex_unlock(&mtx_client);	//UNLOCK

	        split = strtok_r(NULL, " ", &tempptr);

	        UNIQUE_ID = malloc( (strlen(split)+1) * sizeof(char) );
	        strcpy(UNIQUE_ID, split);

	        split = strtok_r(NULL, " ", &tempptr);
	            		
	        DELAY = atoi(split);
		    // if LIST:
		    //      1. open directory
		    //      2. traverse tree
		    //      3. for each record, write entry to socket (morewithls)
	                }
	         // check if messageFromClient is "FETCH"
	        else if(strcmp(split, "FETCH") == 0)
	        {
		    // if fetch:
		    //      1. open file
		    //      2. read file data from hdd
		    //      3. write file to socket
		    //      4. close file
	        }
    }
    close(*myArg); /* parent closes socket to client */

	free(arg);
	pthread_exit(0);
}

int main(int argc, char const *argv[]) 
{
    int a = 0, b = 0, c = 0, i, j, maxWorkerThreadsInServer, bytesRead, server_port, DELAY;
    int *arg;

    char *d = "-d", *p = "-p", *path, *split, *dirorfile, *UNIQUE_ID, copyBuf[buf_SIZE], message[buf_SIZE], buf[buf_SIZE],
    	messageFromClient[buf_SIZE], messageToClient[buf_SIZE], dirName[buf_SIZE], buf_reply[3], buf_OK[] = "OK", buf_PRINTEND[] = "PRINTEND", buf_DONE[] = "DONE";

    ClientInfo *clientStorageArray;

    memset(buf, 0, buf_SIZE);
    memset(copyBuf, 0, buf_SIZE);
    memset(messageFromClient, 0, buf_SIZE);
    memset(messageToClient, 0, buf_SIZE);

    pthread_mutex_init(&mtx_client, 0);

    if (argc == 5)
    {
        for (i = 1; i < 5; i = i + 2) 
        {
            if (strcmp(argv[i], p) == 0)
            {
                server_port = atoi(argv[i + 1]);
            }
            else if (strcmp(argv[i], d) == 0)
            {
                c = 1;
                dirorfile = malloc((strlen(argv[i + 1]) + 1) * sizeof (char));
                strcpy(dirorfile, argv[i + 1]);
            } 
            else 
            {
                if (c == 1)
                    free(dirorfile);

                printf("(content_server) acceptable flags: -p -m -w \n");
                exit(1);
            }
        }
    }

    int arraySize = 1;
    int nextAvailablePos = 0;
    clientStorageArray = malloc( arraySize * sizeof(ClientInfo) );

    /* OPEN Socket */
    int master_sock, client_sock;
    struct sockaddr_in server, client;
    socklen_t clientlen;
    struct sockaddr *serverptr = (struct sockaddr *) &server;
    struct sockaddr *clientptr = (struct sockaddr *) &client;
    struct hostent *rem;

    /* Create socket */
    if ((master_sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) 
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    server.sin_family = AF_INET; /* Internet domain */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(server_port); /* The given port */

    /* Bind socket to address */
    if (bind(master_sock, serverptr, sizeof (server)) < 0) 
    {
        perror("bind");
        return EXIT_FAILURE;
    }

    /* Listen for connections */
    if (listen(master_sock, 5) < 0)
    {
        perror("listen");
        return EXIT_FAILURE;
    }

    printf("Listening for connections to port %d\n", server_port);

    /* MAIN LOOP  */
    int counter = 0;
    while (1)
    {
        clientlen = sizeof (client);

        /* accept connection */
        if ((client_sock = accept(master_sock, clientptr, &clientlen)) < 0)
        {
            perror("accept");
        } 
        else 
        {
        	counter++;
        	printf("counter: %d\n", counter);
        	/* create a thread for each client */
        	if(nextAvailablePos >= arraySize)
        	{
        		printf("realloc\n");
        		arraySize = arraySize * 2;
        		clientStorageArray = realloc(clientStorageArray, arraySize * sizeof(ClientInfo));
        	}

        	arg = malloc(sizeof(int));
        	*arg = client_sock;
        	pthread_create( &(clientStorageArray[nextAvailablePos].client_threadid), NULL, thread_client_function, (void*)arg);
        	nextAvailablePos++;

        	/*
            if ((bytesRead = read(client_sock, messageFromClient, buf_SIZE)) > 0)
            {
            	printf("(content_server) message from mirror manager received: '%s'\n", messageFromClient);

            	char *tempptr = NULL;

                split = strtok_r(messageFromClient, " ", &tempptr);

                // check if messageFromClient is "LIST"
                if(strcmp(split, "LIST") == 0)
            	{
            		write(client_sock, "got your message mate", buf_SIZE);

            		split = strtok_r(NULL, " ", &tempptr);

            		UNIQUE_ID = malloc( (strlen(split)+1) * sizeof(char) );
            		strcpy(UNIQUE_ID, split);

            		split = strtok_r(NULL, " ", &tempptr);
            		
            		DELAY = atoi(split);
	                // if LIST:
	                //      1. open directory
	                //      2. traverse tree
	                //      3. for each record, write entry to socket (morewithls)
                }
                 // check if messageFromClient is "FETCH"
                else if(strcmp(split, "FETCH") == 0)
                {
	                // if fetch:
	                //      1. open file
	                //      2. read file data from hdd
	                //      3. write file to socket
	                //      4. close file
                }
            }
            close(client_sock); //parent closes socket to client
            */

        }
    }

    for (i = 0; i < nextAvailablePos; i++) 
    {
        pthread_join( (clientStorageArray[i].client_threadid), NULL);
    }


    close(master_sock);

    return EXIT_SUCCESS;
}