#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <stdint.h>
#include <pthread.h>
#include "jobScheduler.h"



// Queue * queue;


Job::Job(int newid,int newqueryLen,char **newquery):id(newid),queryLen(newqueryLen) {
    query = (char **) malloc(sizeof(char *)*newqueryLen);
    for (int i = 0; i < newqueryLen; i++) {
        query[i] = (char *) malloc(sizeof(char)*(strlen(newquery[i]) + 1));
        strcpy(query[i],newquery[i]);
    }
}

Job::~Job() {
    for (int i = 0; i < queryLen; i++) {
        free(query[i]);
    }
    free(query);
}

Queue::Queue():start(0),end(-1),count(0) {}

Queue::~Queue(){
    for (int i = 0; i < count; i++) {
        delete data[start+i];
    }
}


JobScheduler::JobScheduler( int newexecution_threads):execution_threads(newexecution_threads) {
    queue = new Queue();
    if((tids = (pthread_t*) malloc(newexecution_threads * sizeof(pthread_t))) == NULL) {
        perror("malloc");
        exit(1);
    }
}

JobScheduler::~JobScheduler(){
    delete queue;
    free(tids);
}

void JobScheduler::submit_job(Job* data) {
    pthread_mutex_lock(&mtx);
    while (queue->count >= POOL_SIZE) {
        pthread_cond_wait(&cond_nonfull, &mtx);
    }
    queue->end = (queue->end + 1) % POOL_SIZE;
    queue->data[queue->end] = data;
    queue->count++;
    pthread_cond_broadcast(&cond_nonempty);
    pthread_mutex_unlock(&mtx);
}

void *execute(void *arg){
    char *temp;
    return temp;
}

void JobScheduler::execute_all_jobs() {
    int err;
    for (int i = 0; i < execution_threads; i++) {
        if((err = pthread_create(tids+i,NULL,execute,(void *)&err))) {
            perror("pthread_create");
            exit(1);
        }
    }
}

void JobScheduler::wait_all_tasks_finish() { //waits all submitted tasks to finish
    int err;
    for (int i = 0; i < execution_threads; i++) {
        if ((err = pthread_join(*(tids + i), NULL))) { // Wait for thread termination
            perror("pthread_join");
            exit(1);
        }
    }
}


Job * JobScheduler::obtain() {
    Job * data = NULL;
    pthread_mutex_lock(&mtx);
    while (queue->count <= 0) {
        pthread_cond_wait(&cond_nonempty, &mtx);
    }
    data = queue->data[queue->start];
    queue->start = (queue->start + 1) % POOL_SIZE;
    queue->count--;
    pthread_cond_broadcast(&cond_nonfull);
    pthread_mutex_unlock(&mtx);
    return data;
}
