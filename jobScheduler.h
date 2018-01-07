#include "heap.h"
#include "trie.h"
#define POOL_SIZE 50

extern pthread_mutex_t head_mtx;
extern pthread_mutex_t queue_mtx;
extern pthread_mutex_t heap_mtx;
extern pthread_mutex_t printer_mtx;
extern pthread_cond_t cond_nonempty;
extern pthread_cond_t cond_nonfull;

class Job {
public:
    int id;
    int queryLen;
    char **query;

    Job(int newid,int newqueryLen,char **newquery);
    ~Job();
};

class Queue {
public:
    Job * data[POOL_SIZE];
    int start;
    int end;
    int count;

    Queue();
    ~Queue();
};

class JobScheduler{
public:
    int execution_threads; // number of execution threads
    Queue* queue; // a queue that holds submitted jobs / tasks
    pthread_t* tids; // execution threads

    JobScheduler(int execution_threads);
    ~JobScheduler();
    void submit_job(Job* j);
    void execute_all_jobs(int type);
    void wait_all_tasks_finish(); //waits all submitted tasks to finish
    Job * obtain();
};

class Parameter{
public:
    Head *head;
    MaxHeap *heap;
    JobScheduler *scheduler;
    char **printer;

    Parameter(Head *newhead,MaxHeap *newheap,JobScheduler *newscheduler);
    ~Parameter();
};

extern Parameter *threadParameter;
