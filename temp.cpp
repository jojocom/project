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

    pthread_mutex_init(&mtx, 0);
    pthread_cond_init(&cond_nonempty, 0);
    pthread_cond_init(&cond_nonfull, 0);


    char **query = (char **) malloc(sizeof(char *)*2);
    query[0] = (char *) malloc(sizeof(char)*(strlen("garcia")+1));
    strcpy(query[0],"garcia");
    query[1] = (char *) malloc(sizeof(char)*(strlen("above")+1));
    strcpy(query[1],"above");

    // sch->execute_all_jobs();

    Job *job = new Job(1,2,query);
    sch->submit_job(job);

    Job *test;
    test = sch->obtain();

    delete sch;

    return(0);
}
