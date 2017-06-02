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


#include "definitions_server.h"
//#include "status.h"

int nextAvailablePos = 0; //queue
pthread_mutex_t mtx;
pthread_cond_t cond_nonempty;
pthread_cond_t cond_nonfull;

// manager
void * thread_manager_function(void *arg) 
{ // arg: "IP:PORT:USERFILE:DELAY"
	char *split, *IP, *PORT, *USERFILE, *UNIQUE_ID="nothingyet";
    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr*) &server;
    struct hostent *rem;

    ThreadArg_manager *myArg = (ThreadArg_manager *)arg;	//giati prepei na ginei auto? (pera tou oti an de ginei xtipaei o compiler -> "dereferencing void * pointer" stin printf)
    QueueEntry *queue;

    int bytesRead, DELAY;

    char messageBuf[buf_SIZE];
    char messageFromContentServer[buf_SIZE];

    memset(messageBuf, 0, buf_SIZE);
    memset(messageFromContentServer, 0, buf_SIZE);
   // sleep(3);
    printf("(mirror_manager) job: %s\n", myArg -> split);
    // once:
    //      Step 1: split "arg"  (find: IP PORT USERFILE DELAY)

    char * tempptr = NULL;

    split = strtok_r(myArg -> split, ":", &tempptr);

    if(split != NULL)
    {
   		IP = malloc( (strlen(split)+1) * sizeof(char) );
   		strcpy(IP, split);

    	split = strtok_r(NULL, ":", &tempptr);

    	PORT = malloc( (strlen(split)+1) * sizeof(char) );
   		strcpy(PORT, split);

   		split = strtok_r(NULL, ":", &tempptr);

    	USERFILE = malloc( (strlen(split)+1) * sizeof(char) );
   		strcpy(USERFILE, split);

   		split = strtok_r(NULL, ":", &tempptr);

    	DELAY = atoi(split);
    }
    else
    {
    	printf("(mirror_manager) no job given OR incorrect job format\n");
    	pthread_exit(0);
    }

//
    int sock_fd = 0;

    //      Step 2: connect to IP:PORT
    /* ________OPEN SOCKET________ */
    if ((sock_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) 
    {
        perror("socket");
        pthread_exit(0);
    }

    /* Find server address */
    if ((rem = gethostbyname(IP)) == NULL) 
    {
        herror("gethostbyname");
        pthread_exit(0);
    }

    int port = atoi(PORT); /*Convert port number to integer*/

    server.sin_family = AF_INET; /* Internet domain */
    memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
    server.sin_port = htons(port); /* Server port */

    /* Initiate connection */
    if (connect(sock_fd, serverptr, sizeof (server)) < 0) 
    {
        herror("connect");
        pthread_exit(0);
    }

    printf("Connecting to IP:%s - PORT: %s \n", IP, PORT);

    //      Step 3: send "LIST UNIQUEID DELAY"
    sprintf(messageBuf, "LIST %s %d", UNIQUE_ID, DELAY);

    write(sock_fd, messageBuf, buf_SIZE);

    //      loop until no data available in socket:
    while (1) 
    {
        //          Step 4: receive a filename from socket
        if ((bytesRead = read(sock_fd, messageFromContentServer, buf_SIZE)) > 0) 
        {
            if (strcmp(messageFromContentServer, "PRINTEND") == 0) 
            {
                memset(messageFromContentServer, 0, buf_SIZE);
                break;
            } 
            else 
            {
            	printf("(mirror_manager) message from content server received: '%s'\n", messageFromContentServer);
                //          Step 5: filter (check if filename is in USERFILE
                // ...
                // ...




                char * filepath = messageFromContentServer;
                char * userfilepath = USERFILE;

                //          Step 6: if match, produce

                //mono produce 
                pthread_mutex_lock(&mtx);

                printf("(manager) entered critical section\n");

                printf("placing element {%s, %s, %s, %s } in queue...\n", IP, PORT, USERFILE, UNIQUE_ID);
                while(nextAvailablePos >= queueSize)
                {
                	printf(">> Queue is full \n");
                	pthread_cond_wait(&cond_nonfull, &mtx);
                }


                queue = myArg -> queue;

                queue[nextAvailablePos].IP = malloc( (strlen(IP)+1) * sizeof(char) );
                strcpy(queue[nextAvailablePos].IP, IP);

                queue[nextAvailablePos].PORT = malloc( (strlen(PORT)+1) * sizeof(char) );
                strcpy(queue[nextAvailablePos].PORT, PORT);

                queue[nextAvailablePos].REMOTEFILEPATH = malloc( (strlen(USERFILE)+1) * sizeof(char) );
                strcpy(queue[nextAvailablePos].REMOTEFILEPATH, USERFILE);

                queue[nextAvailablePos].UNIQUE_ID = malloc( (strlen(UNIQUE_ID)+1) * sizeof(char) );
                strcpy(queue[nextAvailablePos].UNIQUE_ID, UNIQUE_ID);

                nextAvailablePos++;

                pthread_mutex_unlock(&mtx);

                pthread_cond_signal(&cond_nonempty);

                usleep(300000);
            }
        }
    }

    close(sock_fd);
    pthread_exit(0);
}


// worker
void * thread_worker_function(void *arg)
{
    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr*) &server;
    struct hostent *rem;

    int bytesRead, sock_fd;

    char messageBuf[buf_SIZE];
    char messageFromContentServer[buf_SIZE];
    char *split, *IP, *PORT, *REMOTEPATH, *UNIQUE_ID;

    ThreadArg_worker *myArg = (ThreadArg_worker *)arg;
    QueueEntry *queue;

    memset(messageBuf, 0, buf_SIZE);
    memset(messageFromContentServer, 0, buf_SIZE);

    //printf("(worker) job: %s\n", myArg -> split);

    // Loop forever:

    while (1) 
    {
        // Step 1: consume <IP PORT REMOTEFILEPATH>
    	pthread_mutex_lock(&mtx);

    	printf("(worker) entered critical section\n");

        while(nextAvailablePos == 0)
        {
        	printf(">> Queue is empty \n");
        	pthread_cond_wait(&cond_nonempty, &mtx);
        }

        queue = myArg -> queue;

        IP = malloc( (strlen(queue[nextAvailablePos-1].IP)+1) * sizeof(char) );
        strcpy(IP, queue[nextAvailablePos-1].IP);
        free(queue[nextAvailablePos-1].IP);
        queue[nextAvailablePos-1].IP = NULL;

        PORT = malloc( (strlen(queue[nextAvailablePos-1].PORT)+1) * sizeof(char) );
        strcpy(PORT, queue[nextAvailablePos-1].PORT);
        free(queue[nextAvailablePos-1].PORT);
        queue[nextAvailablePos-1].PORT = NULL;

        REMOTEPATH = malloc( (strlen(queue[nextAvailablePos-1].REMOTEFILEPATH)+1) * sizeof(char) );
        strcpy(REMOTEPATH, queue[nextAvailablePos-1].REMOTEFILEPATH);
        free(queue[nextAvailablePos-1].REMOTEFILEPATH);
        queue[nextAvailablePos-1].REMOTEFILEPATH = NULL;

        UNIQUE_ID = malloc( (strlen(queue[nextAvailablePos-1].UNIQUE_ID)+1) * sizeof(char) );
        strcpy(UNIQUE_ID, queue[nextAvailablePos-1].UNIQUE_ID);
        free(queue[nextAvailablePos-1].UNIQUE_ID);
        queue[nextAvailablePos-1].UNIQUE_ID = NULL;

        printf("obtained element {%s, %s, %s, %s }\n", IP, PORT, REMOTEPATH, UNIQUE_ID);

        nextAvailablePos--;

        pthread_mutex_unlock(&mtx);

        pthread_cond_signal(&cond_nonfull);

        usleep(300000);

        // Step 2: connect to IP:PORT
        /* ________OPEN SOCKET________ */
        if ((sock_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
        {
            perror("socket");
            pthread_exit(0);
        }

        /* Find server address */
        if ((rem = gethostbyname(IP)) == NULL) 
        {
            herror("gethostbyname");
            pthread_exit(0);
        }

        int port = atoi(PORT); /*Convert port number to integer*/

        server.sin_family = AF_INET; /* Internet domain */
        memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
        server.sin_port = htons(port); /* Server port */

        /* Initiate connection */
        if (connect(sock_fd, serverptr, sizeof (server)) < 0) 
        {
            herror("connect");
            pthread_exit(0);
        }

        // Step 3: send "FETCH ID REMOTEFILEPATH"
        sprintf(messageBuf, "FETCH %s %s", UNIQUE_ID, REMOTEPATH);

        write(sock_fd, messageBuf, buf_SIZE);
        
        // Step 4: translate remote path to local path
        
        // Step 5: create local file
        
        // Step 6: read data from socket and store to file
                        
        // Step 7: disconnect
        close(sock_fd);
    }

    return EXIT_SUCCESS;
}
















int main(int argc, char const *argv[]) 
{
    int a = 0, b = 0, c = 0, i, j, maxWorkerThreadsInServer, bytesRead, numOfJobs, status, exit_status, poolCounter, server_port;

    char *w = "-w", *m = "-m", *p = "-p", *path, *split, *split2, **next;
    char buf[buf_SIZE], copyBuf[buf_SIZE], message[buf_SIZE], messageFromClient[buf_SIZE], 
        messageToClient[buf_SIZE], dirName[buf_SIZE], jobPath[buf_SIZE], buf_reply[3], buf_OK[] = "OK", buf_PRINTEND[] = "PRINTEND", buf_DONE[] = "DONE";

    ManagerInfo *managerStorageArray;
    WorkerInfo *workerStorageArray;
    QueueEntry *queue;
    ThreadArg_manager *argArray_manager;
    ThreadArg_worker *argArray_worker;

    memset(jobPath, 0, buf_SIZE);

    memset(buf, 0, buf_SIZE);
    memset(copyBuf, 0, buf_SIZE);

    memset(messageFromClient, 0, buf_SIZE);
    memset(messageToClient, 0, buf_SIZE);

    if (argc == 7) 
    {
        for (i = 1; i < 7; i = i + 2) 
        {
            if (strcmp(argv[i], p) == 0)
            {
                server_port = atoi(argv[i + 1]);
            } 
            else if (strcmp(argv[i], m) == 0) 
            {
                c = 1;
                path = malloc((strlen(argv[i + 1]) + 1) * sizeof (char));
                strcpy(path, argv[i + 1]);
            } 
            else if (strcmp(argv[i], w) == 0) 
            {
                maxWorkerThreadsInServer = atoi(argv[i + 1]);
            } 
            else 
            {
                if (c == 1)
                    free(path);

                printf("(mirror server) acceptable flags: -p -m -w \n");
                exit(1);
            }
        }
    }

    printf("port: %d, dirname: %s, threadnum: %d\n", server_port, path, maxWorkerThreadsInServer);
    workerStorageArray = malloc(maxWorkerThreadsInServer * sizeof (WorkerInfo));

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

    /* CREATE QUEUE */
    queue = malloc( queueSize * sizeof(QueueEntry) ); //queueSize: 100

    /* CREATE WORKER THREADS */
    argArray_worker = malloc( maxWorkerThreadsInServer * sizeof(ThreadArg_worker) );
    for (i = 0; i < maxWorkerThreadsInServer; i++) 
    {
    	argArray_worker[i].queue = queue;
        pthread_create( &(workerStorageArray[i].worker_threadid), NULL, thread_worker_function, (void *)(argArray_worker+i) );
    }
    


    /* MAIN LOOP  */

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
            if ((bytesRead = read(client_sock, buf, buf_SIZE)) > 0) 
            {
                printf("(mirror_server) buf: %s\n", buf);

                // -------------------------------------------------------- //
                numOfJobs = 0;

                char * tempptr = NULL;

                strcpy(copyBuf, buf);

                split = strtok_r(copyBuf, ",", &tempptr);

                while(split != NULL)
                {
                	numOfJobs++;
                	split = strtok_r(NULL, ",", &tempptr);
                }
                memset(copyBuf, 0, buf_SIZE);

                managerStorageArray = malloc( numOfJobs * sizeof(ManagerInfo) );

                // -------------------------------------------------------- //
                argArray_manager = malloc( numOfJobs * sizeof(ThreadArg_manager) );

                tempptr = NULL;

                split2 = strtok_r(buf, ",", &tempptr);

                for(j=0; j<numOfJobs; j++) // i know i have numOfJobs jobs
                {
                    printf("(mirror_server) job: %s\n", split2);

                    argArray_manager[j].queue = queue;
                    argArray_manager[j].split = split2;

                    /* for each job create a mirror manager thread */
                    pthread_create( &(managerStorageArray[j].manager_threadid), NULL, thread_manager_function, (void*)(argArray_manager+j) );

                    /* go to the next job */
                    split2 = strtok_r(NULL, ",", &tempptr);
                }

                memset(messageToClient, 0, buf_SIZE);
                write(client_sock, buf_PRINTEND, 9);
            }
            close(client_sock); /* parent closes socket to client */
        }
    }

    close(master_sock);

    free(workerStorageArray);
    free(argArray_manager);
    free(argArray_worker);
    free(queue);

    return EXIT_SUCCESS;
}