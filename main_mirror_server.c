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

// manager

void * thread_manager_function(void * arg) 
{ // arg: "IP:PORT:USERFILE:DELAY"
    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr*) &server;
    struct hostent *rem;

    int bytesRead;

    char messageBuf[buf_SIZE];
    char messageFromContentServer[buf_SIZE];

    // once:
    //      Step 1: split "arg"  (find: IP PORT USERFILE DELAY)
    char * IP;
    char * PORT;
    char * USERFILE;
    char * DELAY;
    char * UNIQUE_ID; // autogenerate

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

    printf("Connecting to '%s' port '%s' \n", IP, PORT);

    //      Step 3: send "LIST UNIQUEID DELAY"
    sprintf(messageBuf, "LIST %s %s", UNIQUE_ID, DELAY);

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
                //          Step 5: filter (check if filename is in USERFILE
                char * filepath = messageFromContentServer;
                char * userfilepath = USERFILE;

                //          Step 6: if match, produce
            }
        }
    }

    close(sock_fd);
    pthread_exit(0);
}


// worker

void * thread_worker_function(void * arg)
{
    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr*) &server;
    struct hostent *rem;

    int bytesRead, sock_fd;

    char messageBuf[buf_SIZE];
    char messageFromContentServer[buf_SIZE];
    char *split;

    // Loop forever:

    while (1) 
    {
        // Step 1: consume <IP PORT REMOTEFILEPATH>
        char * IP;
        char * PORT;
        char * REMOTEPATH;
        char * UNIQUE_ID;

        // Step 2: connect to IP:PORT

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
    int a = 0, b = 0, c = 0, i, j, maxWorkerThreadsInServer, bytesRead, status, exit_status, poolCounter, nextAvailablePos, server_port;

    char *w = "-w", *m = "-m", *p = "-p", *path, *split, **next;
    char buf[buf_SIZE], copyBuf[buf_SIZE], message[buf_SIZE], messageFromClient[buf_SIZE], 
        messageToClient[buf_SIZE], dirName[buf_SIZE], jobPath[buf_SIZE], buf_reply[3], buf_OK[] = "OK", buf_PRINTEND[] = "PRINTEND", buf_DONE[] = "DONE";

    WorkerInfo *serverStorageArray;
    QueueEntry *queue;
    //    jobInfo *poolStorageArray;

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
    serverStorageArray = malloc(maxWorkerThreadsInServer * sizeof (WorkerInfo));

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
    /*
    for (i = 0; i < maxWorkerThreadsInServer; i++) 
    {
        pthread_create(&(serverStorageArray[i].worker_threadid), NULL, thread_worker_function, NULL);
    }
    */


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

                strcpy(copyBuf, buf);

                char * tempptr = NULL;

                split = strtok_r(buf, ",", &tempptr);

                while (split != NULL) 
                {
                    printf("(mirror_server) processing token: %s\n", split);
                    split = strtok_r(NULL, ",", &tempptr);
                }

                memset(messageToClient, 0, buf_SIZE);
                write(client_sock, buf_PRINTEND, 9);
            }
            close(client_sock); /* parent closes socket to client */
        }
    }

    close(master_sock);

    free(serverStorageArray);
    free(queue);

    return EXIT_SUCCESS;
}