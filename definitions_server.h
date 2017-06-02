
#ifndef DEFINITIONS_SERVER_H
#define DEFINITIONS_SERVER_H

#define fileBuf_SIZE 1024
#define userBuf_SIZE 1024
#define queueSize 100
#define buf_SIZE 1024

typedef struct QueueEntry
{
	char *IP;
	char *PORT;
	char *REMOTEFILEPATH;
	char *UNIQUE_ID;
} QueueEntry;

typedef struct ManagerInfo
{
	pthread_t manager_threadid;
}ManagerInfo;

typedef struct WorkerInfo 
{
    pthread_t worker_threadid;
} WorkerInfo;

typedef struct ThreadArg_manager
{
	QueueEntry *queue;
	char *split;
}ThreadArg_manager;

typedef struct ThreadArg_worker
{
	QueueEntry *queue;
	//char *split;
}ThreadArg_worker;

#endif /* DEFINITIONS_H */

