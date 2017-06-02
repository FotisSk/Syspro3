#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/types.h>      /* sockets */
#include <sys/socket.h>      /* sockets */
#include <netinet/in.h>      /* internet sockets */
#include <unistd.h>          /* read, write, close */
#include <netdb.h>          /* gethostbyaddr */
#include <stdlib.h>          /* exit */
#include <string.h>          /* strlen */
#include <stdio.h>

#include "definitions_client.h"

int main(int argc, char const *argv[]) 
{
    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr*) &server;
    struct hostent *rem;

    int port, i, a = 0, b = 0, c = 0, sock_fd, bytesRead;
    char userBuf[userBuf_SIZE], messageFromServer[buf_SIZE], buf_OK[] = "OK";
    char *p = "-p", *n = "-n", *s = "-s", *server_address, *server_port, *jobs;

    if (argc == 7) //diavasma apo arxeio
    {
        for (i = 1; i < 7; i = i + 2) {
            if (strcmp(argv[i], n) == 0) {
                a = 1;
                server_address = malloc((strlen(argv[i + 1]) + 1) * sizeof (char));
                strcpy(server_address, argv[i + 1]);
            } else if (strcmp(argv[i], p) == 0) {
                b = 1;
                server_port = malloc((strlen(argv[i + 1]) + 1) * sizeof (char));
                strcpy(server_port, argv[i + 1]);
            } else if (strcmp(argv[i], s) == 0) {
                c = 1;
                jobs = malloc((strlen(argv[i + 1]) + 1) * sizeof (char));
                strcpy(jobs, argv[i + 1]);
            } else {
                if (a == 1)
                    free(server_address);
                if (b == 1)
                    free(server_port);
                if (c == 1)
                    free(jobs);

                printf("(mirror_initiator) acceptable flags: -n -p -s\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    /* ________OPEN SOCKET________ */
    if ((sock_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    /* Find server address */
    if ((rem = gethostbyname(server_address)) == NULL) 
    {
        herror("gethostbyname");
        return EXIT_FAILURE;
    }

    port = atoi(server_port); /*Convert port number to integer*/

    printf("sock_fd: %d\n", sock_fd);
    printf("MirrorServerAddress: %s, MirrorServerPort: %d, task: %s\n", server_address, port, jobs);

    server.sin_family = AF_INET; /* Internet domain */
    memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
    server.sin_port = htons(port); /* Server port */

    /* Initiate connection */
    if (connect(sock_fd, serverptr, sizeof(server)) < 0) 
    {
        herror("connect");
        return EXIT_FAILURE;
    }

    printf("Connecting to IP: %s - PORT: %d\n", argv[2], port);

    // ------------------------------------------------

    memset(messageFromServer, 0, buf_SIZE);
    memset(userBuf, 0, userBuf_SIZE);

    strcpy(userBuf, jobs);

    /* ________WRITE TO FIFO________ */
    int l = strlen(userBuf);
    if (userBuf[l - 1] == '\n') 
    {   //exclude '\n'
        userBuf[l - 1] = '\0';
    }
    write(sock_fd, userBuf, buf_SIZE);

    memset(userBuf, 0, userBuf_SIZE);

    while (1) 
    {
        if ((bytesRead = read(sock_fd, messageFromServer, buf_SIZE)) > 0)
        {
            if (strcmp(messageFromServer, "PRINTEND") == 0)
             {
                printf("\n");
                memset(messageFromServer, 0, buf_SIZE);
                break;
            } 
            else 
            {
                write(sock_fd, buf_OK, 3);
                printf("%s\n", messageFromServer);
                memset(messageFromServer, 0, buf_SIZE);
            }
        }
    }
    /**********************************************************************************************/

    close(sock_fd);

    free(server_address);
    free(server_port);
    free(jobs);

    return EXIT_SUCCESS;
}