#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include "list.h"
#include "hash_functions.h"
#include "jobScheduler.h"

pthread_mutex_t head_mtx;
pthread_mutex_t queue_mtx;
pthread_mutex_t heap_mtx;
pthread_mutex_t printer_mtx;
pthread_cond_t cond_empty;
pthread_cond_t cond_nonempty;
pthread_cond_t cond_nonfull;
Parameter *threadParameter;

Job::Job(int newid,int newqueryLen,char **newquery,int newPrevCommands):id(newid),queryLen(newqueryLen),prevCommands(newPrevCommands) {
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

Queue::Queue():start(0),end(-1),count(0),todo(0) {
    for (int i = 0; i < POOL_SIZE; i++) {
        data[i] = NULL;
    }
}

Queue::~Queue(){}

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
    pthread_mutex_lock(&queue_mtx);
    while (queue->count >= POOL_SIZE) {
        pthread_cond_wait(&cond_nonfull, &queue_mtx);
    }
    queue->end = (queue->end + 1) % POOL_SIZE;
    queue->data[queue->end] = data;
    queue->count++;
    pthread_cond_broadcast(&cond_nonempty);
    pthread_mutex_unlock(&queue_mtx);
}

void *executeDynamic(void *arg){
    // std::cout << "thread" << '\n';
    Head *head = threadParameter->head;
    JobScheduler *scheduler = threadParameter->scheduler;

    while(1){
        Job *job = scheduler->obtain();     // obtain from scheduler's queue
        MaxHeap *heap = threadParameter->heap;
        char **printer = threadParameter->printer;
        if (job == NULL) {
            cout << "null" << endl;
        }
        int id = job->id;
        char **query = job->query;
        int whitespace = job->queryLen - 1;
        int prevCommands = job->prevCommands;
        if(strcmp(query[0],"poison") == 0){
            return NULL;
        }

        uint32_t bitArray[M];
        for (int i = 0; i < M; i++) {
            bitArray[i] = 0;
        }
        char **queryStart = query + 1;

        if(strcmp(query[0],"Q") == 0){              // for Q query
            // std::cout << "after Q" << '\n';
            int found = 0;
            for (int k = 0; k < whitespace; k++) {
                char **temp = queryStart + k;
                for (int j = 1; j <= whitespace-k; j++) {
                    // std::cout << "before searchNgram" << '\n';
                    int x = searchNgram(temp,j,head,id);
                    // std::cout << "x: " << x << '\n';
                    if(x == 1){
                        int length = 0;
                        for (int d = 0; d < j; d++) {
                            length += strlen(temp[d]);
                        }
                        char *mykey;
                        mykey = (char *) malloc(sizeof(char)*(length + j));
                        for (int d = 0; d < j; d++) {
                            if(d == 0){
                                strcpy(mykey,temp[d]);
                            }
                            else{
                                strcat(mykey,temp[d]);
                            }
                            if(d != j-1){
                                strcat(mykey," ");
                            }
                        }
                        int hash1 = murmurhash(mykey,(uint32_t) strlen(mykey),0) % (M*32);
                        int hash2 = hash_pearson(mykey) % (M*32);
                        int hash3 = hash_jenkins(mykey) % (M*32);

                        if((bitChecker(bitArray[hash1/32],hash1%32) == 0) || (bitChecker(bitArray[hash2/32],hash2%32) == 0) || (bitChecker(bitArray[hash3/32],hash3%32) == 0) || (bitChecker(bitArray[((hash3+hash1)%(M*32))/32],((hash3+hash1)%(M*32))%32) == 0) || (bitChecker(bitArray[((hash2+hash1)%(M*32))/32],((hash2+hash1)%(M*32))%32) == 0)){
                            bitArray[hash1/32] = bitChanger(bitArray[hash1/32],hash1%32);
                            bitArray[hash2/32] = bitChanger(bitArray[hash2/32],hash2%32);
                            bitArray[hash3/32] = bitChanger(bitArray[hash3/32],hash3%32);
                            bitArray[((hash2+hash1)%(M*32))/32] = bitChanger(bitArray[((hash2+hash1)%(M*32))/32],((hash2+hash1)%(M*32))%32);
                            bitArray[((hash3+hash1)%(M*32))/32] = bitChanger(bitArray[((hash3+hash1)%(M*32))/32],((hash3+hash1)%(M*32))%32);
                            // std::cout << "before printer" << '\n';
                            if(found == 0){
                                printer[id - prevCommands] = (char *) malloc(sizeof(char)*(strlen(mykey)+1));
                                strcpy(printer[id - prevCommands],mykey);
                            } else{
                                int currentLen = strlen(printer[id - prevCommands]);
                                printer[id - prevCommands] = (char *) realloc(printer[id - prevCommands], sizeof(char)*(currentLen+strlen(mykey)+2));
                                strcat(printer[id - prevCommands],"|");
                                strcat(printer[id - prevCommands],mykey);
                            }
                            // std::cout << "after printer" << '\n';
                            pthread_mutex_lock(&heap_mtx);
                            heap->insertKey(mykey);
                            pthread_mutex_unlock(&heap_mtx);
                        }
                        free(mykey);
                        found = 1;
                    } else if(x == 0){
                        break;
                    }
                }
            }
            if(found == 0){     // if there are no Ngrams int the query
                printer[id - prevCommands] = (char *) malloc(sizeof(char)*(3));
                strcpy(printer[id - prevCommands],"-1");
            }
        }
        pthread_mutex_lock(&queue_mtx);
        scheduler->queue->todo--;
        if (scheduler->queue->todo == 0) {
            pthread_cond_signal(&cond_empty);
        }
        pthread_mutex_unlock(&queue_mtx);
    }

    return NULL;
}

void *executeStatic(void *arg){
    Head *head = threadParameter->head;
    JobScheduler *scheduler = threadParameter->scheduler;

    while(1){
        Job *job = scheduler->obtain();     // obtain from scheduler's queue
        MaxHeap *heap = threadParameter->heap;
        char **printer = threadParameter->printer;
        if (job == NULL) {
            cout << "null" << endl;
        }
        int id = job->id;
        char **query = job->query;
        int whitespace = job->queryLen - 1;
        if(strcmp(query[0],"poison") == 0){
            return NULL;
        }

        uint32_t bitArray[M];
        for (int i = 0; i < M; i++) {
            bitArray[i] = 0;
        }
        char **queryStart = query + 1;

        if(strcmp(query[0],"Q") == 0){              // for Q query
            int found = 0;
            for (int k = 0; k < whitespace; k++) {
                char **temp = queryStart + k;
                for (int j = 1; j <= whitespace-k; j++) {
                    int x = searchNgramStatic(temp,j,head);
                    if(x == 1){
                        int length = 0;
                        for (int d = 0; d < j; d++) {
                            length += strlen(temp[d]);
                        }
                        char *mykey;
                        mykey = (char *) malloc(sizeof(char)*(length + j));
                        for (int d = 0; d < j; d++) {
                            if(d == 0){
                                strcpy(mykey,temp[d]);
                            }
                            else{
                                strcat(mykey,temp[d]);
                            }
                            if(d != j-1){
                                strcat(mykey," ");
                            }
                        }
                        int hash1 = murmurhash(mykey,(uint32_t) strlen(mykey),0) % (M*32);
                        int hash2 = hash_pearson(mykey) % (M*32);
                        int hash3 = hash_jenkins(mykey) % (M*32);

                        if((bitChecker(bitArray[hash1/32],hash1%32) == 0) || (bitChecker(bitArray[hash2/32],hash2%32) == 0) || (bitChecker(bitArray[hash3/32],hash3%32) == 0) || (bitChecker(bitArray[((hash3+hash1)%(M*32))/32],((hash3+hash1)%(M*32))%32) == 0) || (bitChecker(bitArray[((hash2+hash1)%(M*32))/32],((hash2+hash1)%(M*32))%32) == 0)){
                            bitArray[hash1/32] = bitChanger(bitArray[hash1/32],hash1%32);
                            bitArray[hash2/32] = bitChanger(bitArray[hash2/32],hash2%32);
                            bitArray[hash3/32] = bitChanger(bitArray[hash3/32],hash3%32);
                            bitArray[((hash2+hash1)%(M*32))/32] = bitChanger(bitArray[((hash2+hash1)%(M*32))/32],((hash2+hash1)%(M*32))%32);
                            bitArray[((hash3+hash1)%(M*32))/32] = bitChanger(bitArray[((hash3+hash1)%(M*32))/32],((hash3+hash1)%(M*32))%32);

                            if(found == 0){
                                printer[id] = (char *) malloc(sizeof(char)*(strlen(mykey)+1));
                                strcpy(printer[id],mykey);
                            } else{
                                int currentLen = strlen(printer[id]);
                                printer[id] = (char *) realloc(printer[id], sizeof(char)*(currentLen+strlen(mykey)+2));
                                strcat(printer[id],"|");
                                strcat(printer[id],mykey);
                            }
                            pthread_mutex_lock(&heap_mtx);
                            heap->insertKey(mykey);
                            pthread_mutex_unlock(&heap_mtx);
                        }
                        free(mykey);
                        found = 1;
                    } else if(x == 0){
                        break;
                    }
                }
            }
            if(found == 0){     // if there are no Ngrams int the query
                printer[id] = (char *) malloc(sizeof(char)*(3));
                strcpy(printer[id],"-1");
            }
        }
        pthread_mutex_lock(&queue_mtx);
        scheduler->queue->todo--;
        if (scheduler->queue->todo == 0) {
            pthread_cond_signal(&cond_empty);
        }
        pthread_mutex_unlock(&queue_mtx);
    }

    return NULL;
}

void JobScheduler::execute_all_jobs(int type) {
    int err;
    for (int i = 0; i < execution_threads; i++) {
        if( type == 0){
            if((err = pthread_create(tids+i,NULL,executeDynamic,(void *)threadParameter))) {
                perror("pthread_create");
                exit(1);
            }
        } else{
            if((err = pthread_create(tids+i,NULL,executeStatic,(void *)threadParameter))) {
                perror("pthread_create");
                exit(1);
            }
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
    pthread_mutex_lock(&queue_mtx);
    while (queue->count <= 0) {
        pthread_cond_wait(&cond_nonempty, &queue_mtx);
    }
    data = queue->data[queue->start];
    queue->start = (queue->start + 1) % POOL_SIZE;
    queue->count--;
    pthread_cond_broadcast(&cond_nonfull);
    pthread_mutex_unlock(&queue_mtx);
    return data;
}

Parameter::Parameter(Head *newhead,MaxHeap *newheap,JobScheduler *newscheduler):printer(NULL){
    head = newhead;
    heap = newheap;
    scheduler = newscheduler;
}

Parameter::~Parameter(){
    delete scheduler;
    delete heap;
}
