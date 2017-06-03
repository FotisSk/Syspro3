
#ifndef DEFINITIONS_CONTENTSERVER_H
#define DEFINITIONS_CONTENTSERVER_H

#define fileBuf_SIZE 1024
#define userBuf_SIZE 1024
#define buf_SIZE 1024

typedef struct ClientInfo
{
	pthread_t client_threadid;	
}ClientInfo;

typedef struct WorkerInfo 
{
    pthread_t worker_threadid;
} WorkerInfo;


#endif /* DEFINITIONS_H */

