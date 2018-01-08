#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <stdint.h>
// #include <pthread.h>
#include "jobScheduler.h"

using namespace std;

void *threadFunction(void *arg){
    std::cout << "thread" << '\n';
    Head *head = threadParameter->head;
    MaxHeap *heap = threadParameter->heap;
    JobScheduler *scheduler = threadParameter->scheduler;
    char **printer = threadParameter->printer;

    while(1){
        std::cout << "before obtain" << '\n';
        Job *job = scheduler->obtain();
        std::cout << "after obtain" << '\n';
        if (job == NULL) {
            cout << "null" << endl;
        }
        int id = job->id;
        char **query = job->query;
        int whitespace = job->queryLen - 1;
        int prevCommands = job->prevCommands;
        if(strcmp(query[0],"poison") == 0){
            break;
        }
    }
    return NULL;
}

int main () {

    JobScheduler *sch = new JobScheduler(6);
    threadParameter = new Parameter(NULL,NULL,sch);
    cout << sch->execution_threads << endl;
    if (sch->queue == NULL) {
        cout << "NULL" << endl;
    }
    if (sch->tids == NULL) {
        cout << "NULL2" << endl;
    }

    pthread_mutex_init(&queue_mtx, 0);
    pthread_cond_init(&cond_empty, 0);
    pthread_cond_init(&cond_nonempty, 0);
    pthread_cond_init(&cond_nonfull, 0);


    char **query = (char **) malloc(sizeof(char *)*2);
    query[0] = (char *) malloc(sizeof(char)*(strlen("garcia")+1));
    strcpy(query[0],"garcia");
    query[1] = (char *) malloc(sizeof(char)*(strlen("above")+1));
    strcpy(query[1],"above");

    // sch->execute_all_jobs();

    int err;
    for (int i = 0; i < sch->execution_threads; i++) {
            if((err = pthread_create(sch->tids+i,NULL,threadFunction,(void *)threadParameter))) {
                perror("pthread_create");
                exit(1);
            }
    }

    Job *job1 = new Job(1,2,query,0);
    Job *job2 = new Job(1,2,query,0);
    for (int i = 0; i < 50; i++) {
        sch->submit_job(job1);
        sch->submit_job(job2);
    }

    // Job *test1;
    // test1 = sch->obtain();
    //
    // for (int i = 0; i < test1->queryLen; i++) {
    //     cout << test1->query[i] << endl;
    // }
    //
    // delete test1;
    // // cout << "\t1" << endl;
    // Job *test2;
    // // cout << "\t2" << endl;
    // test2 = sch->obtain();
    // // cout << "\t3" << endl;
    //
    // for (int i = 0; i < test2->queryLen; i++) {
    //     cout << test2->query[i] << endl;
    // }
    // // cout << "\t4" << endl;
    // delete test2;


    std::cout << "queue count: " << sch->queue->count << '\n';

    while(sch->queue->count != 0){
        std::cout << "1" << '\n';
        pthread_cond_wait(&cond_empty, &queue_mtx1);
        std::cout << "2" << '\n';
    }

    std::cout << "queue count: " << sch->queue->count << '\n';
    // poisoning threads
    char **poison = (char **) malloc(sizeof(char*));
    poison[0] = (char *) malloc(sizeof(char)*(strlen("poison")+1));
    strcpy(poison[0],"poison");
    Job *poisonJob = new Job(-1,1,poison,0);
    for (int i = 0; i < sch->execution_threads; i++) {
        sch->submit_job(poisonJob);
    }
    // delete poisonJob
    delete poisonJob;
    free(poison[0]);
    free(poison);

    for (int i = 0; i < sch->execution_threads; i++) {
        if ((err = pthread_join(*(sch->tids + i), NULL))) { // Wait for thread termination
            perror("pthread_join");
            exit(1);
        }
    }

    // cout << "\t5" << endl;
    delete sch;
    // cout << "\t6" << endl;
    for (int i = 0; i < 2; i++) {
        free(query[i]);
    }
    free(query);

    pthread_cond_destroy(&cond_empty);
    pthread_cond_destroy(&cond_nonempty);
    pthread_cond_destroy(&cond_nonfull);
    pthread_mutex_destroy(&queue_mtx);

    return(0);
}
