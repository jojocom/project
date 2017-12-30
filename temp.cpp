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



int main () {

    JobScheduler *sch = new JobScheduler(6);
    cout << sch->execution_threads << endl;
    if (sch->queue == NULL) {
        cout << "NULL" << endl;
    }
    if (sch->tids == NULL) {
        cout << "NULL2" << endl;
    }

    pthread_mutex_init(&queue_mtx, 0);
    pthread_cond_init(&cond_nonempty, 0);
    pthread_cond_init(&cond_nonfull, 0);


    char **query = (char **) malloc(sizeof(char *)*2);
    query[0] = (char *) malloc(sizeof(char)*(strlen("garcia")+1));
    strcpy(query[0],"garcia");
    query[1] = (char *) malloc(sizeof(char)*(strlen("above")+1));
    strcpy(query[1],"above");

    // sch->execute_all_jobs();

    Job *job1 = new Job(1,2,query);
    Job *job2 = new Job(1,2,query);
    sch->submit_job(job1);
    sch->submit_job(job2);

    Job *test1;
    test1 = sch->obtain();

    for (int i = 0; i < test1->queryLen; i++) {
        cout << test1->query[i] << endl;
    }

    delete test1;
    // cout << "\t1" << endl;
    Job *test2;
    // cout << "\t2" << endl;
    test2 = sch->obtain();
    // cout << "\t3" << endl;

    for (int i = 0; i < test2->queryLen; i++) {
        cout << test2->query[i] << endl;
    }
    // cout << "\t4" << endl;
    delete test2;
    // cout << "\t5" << endl;
    delete sch;
    // cout << "\t6" << endl;
    for (int i = 0; i < 2; i++) {
        free(query[i]);
    }
    free(query);

    pthread_cond_destroy(&cond_nonempty);
    pthread_cond_destroy(&cond_nonfull);
    pthread_mutex_destroy(&queue_mtx);

    return(0);
}
