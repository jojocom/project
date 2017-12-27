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
    Queue* q; // a queue that holds submitted jobs / tasks
    p_thread_t* tids; // execution threads

    JobScheduler* initialize_scheduler( int execution_threads);
    void submit_job( JobScheduler* sch, Job* j);
    void execute_all_jobs( JobScheduler* sch);
    void wait_all_tasks_finish( JobScheduler* sch); //waits all submitted tasks to finish
    OK_SUCCESS destroy_scheduler( JobScheduler* sch);
};
