#include "aaa.h"

#define POOL_SIZE 20

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

struct JobScheduler{
public:
    int execution_threads; // number of execution threads
    Queue* queue; // a queue that holds submitted jobs / tasks
    pthread_t* tids; // execution threads

    JobScheduler(int execution_threads);
    ~JobScheduler();
    void submit_job(Job* j);
    void execute_all_jobs();
    void wait_all_tasks_finish(); //waits all submitted tasks to finish
    Job * obtain();
};
