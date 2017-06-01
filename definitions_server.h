
#ifndef DEFINITIONS_SERVER_H
#define DEFINITIONS_SERVER_H

#define fileBuf_SIZE 1024
#define userBuf_SIZE 1024
#define queueSize 100
#define buf_SIZE 1024


typedef struct jobInfo {
    int job_PID;
    int job_NUM;
    int job_STATUS; //0:active, 1:finished, 3:suspended
    int startTimeInSeconds;
    int stop;
    int cont;
    int timeActive;
} jobInfo;

typedef struct QueueEntry
{
	char *IP;
	char *PORT;
	char *REMOTEPATH;
	char *UNIQUE_ID;
} QueueEntry;

typedef struct WorkerInfo 
{
    pthread_t worker_threadid;
} WorkerInfo;


#endif /* DEFINITIONS_H */

