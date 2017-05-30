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

int main(int argc, char const *argv[]) 
{
    int a = 0, b = 0, c = 0, i, j, maxWorkerThreadsInServer, bytesRead, status, exit_status, jobCounter, oldSize, timeDuration, firstTime = 0,
            poolCounter, jobID, nextAvailablePos, nextAvailablePos_pool, poolNum, tempReadFd_coord, tempWriteFd_coord, tempReadFd_pool,
            tempWriteFd_pool, jobPID, stderrToFile, stdoutToFile, finishedJobs, jobID2, poolPos, secondsActive, jobID_pool, posInPoolStorage, thisPoolPID, server_port;

    char *w = "-w", *m = "-m", *p = "-p", *fifo_WRITE, *path, *split, *split2, *split3, **next;
    char buf[buf_SIZE], copyBuf[buf_SIZE], copyBuf2[buf_SIZE], copyBuf_pool[buf_SIZE], message[buf_SIZE],
            messageToCoord[buf_SIZE], messageFromPool[buf_SIZE], messageFromClient[buf_SIZE], messageFromCoord[buf_SIZE], messageToPool[buf_SIZE], messageToClient[buf_SIZE],
            poolName_in[buf_SIZE], poolName_out[buf_SIZE], dirName[buf_SIZE], jobPath[buf_SIZE], poolBuf[buf_SIZE], buf_reply[3], buf_OK[] = "OK", buf_PRINTEND[] = "PRINTEND", buf_DONE[] = "DONE";

    memset(jobPath, 0, buf_SIZE);

    memset(buf, 0, buf_SIZE);
    memset(copyBuf, 0, buf_SIZE);

    memset(messageFromClient, 0, buf_SIZE);
    memset(messageToClient, 0, buf_SIZE);

    if (argc == 7)
    { // to be changed
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
                // check if buf is "LIST"
                
                // if LIST:
                //      1. open directory
                //      2. traverse tree
                //      3. for each record, write entry to socket (morewithls)
                
                // check if buf is "FETCH"
                
                // if fetch:
                //      1. open file
                //      2. read file data from hdd
                //      3. write file to socket
                //      4. close file
            }
            close(client_sock); /* parent closes socket to client */
        }
    }

    close(master_sock);

    return EXIT_SUCCESS;
}