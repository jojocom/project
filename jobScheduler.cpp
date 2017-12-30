#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <stdint.h>
#include <pthread.h>
#include "list.h"
#include "hash_functions.h"
#include "jobScheduler.h"

pthread_mutex_t queue_mtx;
pthread_mutex_t heap_mtx;
pthread_cond_t cond_nonempty;
pthread_cond_t cond_nonfull;

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
    return NULL;
}

void *executeStatic(void *arg){
    cout << "thread" << endl;
    Parameter *threadParameter = (Parameter *) arg;

    Head *head = threadParameter->head;
    MaxHeap *heap = threadParameter->heap;
    JobScheduler *scheduler = threadParameter->scheduler;
    // obtain from scheduler's queue
    cout << "a" << endl;
    Job *job = scheduler->obtain();
    cout << "b" << endl;
    cout << "here" << endl;
    if (job == NULL) {
        cout << "null" << endl;
    }
    // char **query = job->query;
    // cout << "here1" << endl;
    // int whitespace = job->queryLen - 1;
    // cout << "here2" << endl;
    //
    // cout << "id: " << job->id << " queryLen: " << job->queryLen << endl;
    // for (int i = 0; i < job->queryLen; i++) {
    //     cout << job->query[i] << " ";
    // }
    // cout << endl;
    //
    // cout << "aaa" << endl;
    //
    // uint32_t bitArray[M];
    // for (int i = 0; i < M; i++) {
    //     bitArray[i] = 0;
    // }
    // char **queryStart = query + 1;
    // if(strcmp(query[0],"Q") == 0){              // for Q query
    //     int found = 0;
    //     for (int k = 0; k < whitespace; k++) {
    //         char **temp = queryStart + k;
    //         for (int j = 1; j <= whitespace-k; j++) {
    //             int x = searchNgramStatic(temp,j,head);
    //             if(x == 1){
    //                 int length = 0;
    //                 for (int d = 0; d < j; d++) {
    //                     length += strlen(temp[d]);
    //                 }
    //                 char *mykey;
    //                 mykey = (char *) malloc(sizeof(char)*(length + j));
    //                 for (int d = 0; d < j; d++) {
    //                     if(d == 0){
    //                         strcpy(mykey,temp[d]);
    //                     }
    //                     else{
    //                         strcat(mykey,temp[d]);
    //                     }
    //                     if(d != j-1){
    //                         strcat(mykey," ");
    //                     }
    //                 }
    //                 int hash1 = murmurhash(mykey,(uint32_t) strlen(mykey),0) % (M*32);
    //                 int hash2 = hash_pearson(mykey) % (M*32);
    //                 int hash3 = hash_jenkins(mykey) % (M*32);
    //
    //                 if((bitChecker(bitArray[hash1/32],hash1%32) == 0) || (bitChecker(bitArray[hash2/32],hash2%32) == 0) || (bitChecker(bitArray[hash3/32],hash3%32) == 0) || (bitChecker(bitArray[((hash3+hash1)%(M*32))/32],((hash3+hash1)%(M*32))%32) == 0) || (bitChecker(bitArray[((hash2+hash1)%(M*32))/32],((hash2+hash1)%(M*32))%32) == 0)){
    //                     bitArray[hash1/32] = bitChanger(bitArray[hash1/32],hash1%32);
    //                     bitArray[hash2/32] = bitChanger(bitArray[hash2/32],hash2%32);
    //                     bitArray[hash3/32] = bitChanger(bitArray[hash3/32],hash3%32);
    //                     bitArray[((hash2+hash1)%(M*32))/32] = bitChanger(bitArray[((hash2+hash1)%(M*32))/32],((hash2+hash1)%(M*32))%32);
    //                     bitArray[((hash3+hash1)%(M*32))/32] = bitChanger(bitArray[((hash3+hash1)%(M*32))/32],((hash3+hash1)%(M*32))%32);
    //
    //                     if(found == 0){
    //                         cout << mykey;
    //                     } else{
    //                         cout << "|";
    //                         cout << mykey;
    //                     }
    //                     heap->insertKey(mykey);
    //                 }
    //                 free(mykey);
    //                 found = 1;
    //             } else if(x == 0){
    //                 break;
    //             }
    //         }
    //     }
    //     if(found == 0){
    //         cout << "-1";               // if there are no Ngrams int the query
    //     }
    //     cout << endl;
    // }
    // else if (strcmp(query[0],"F") == 0){
    //     if(whitespace != 0){
    //         int k = atoi(query[1]);
    //         cout << "Top: ";
    //         for (int i = 0; i < k; i++) {
    //             Element *el1 = heap->extractMax();
    //             if(el1 != NULL){
    //                 if( i == k-1){
    //                     cout << el1->word << endl;
    //                 } else{
    //                     cout << el1->word << "|";
    //                 }
    //                 if(heap->elements != 0){
    //                     delete el1;
    //                 }
    //             }
    //         }
    //         delete heap;
    //         heap = new MaxHeap(HeapCap);
    //     } else{
    //         delete heap;
    //         heap = new MaxHeap(HeapCap);
    //     }
    // }
    // else if (strcmp(query[0],"P") == 0){
    //     // printAll(head->root,0);
    // }
    //
    // for(int i = 0; i <= whitespace; i++){
    //     free(query[i]);
    // }

    return NULL;
}

void JobScheduler::execute_all_jobs(int type,Head *head,MaxHeap *heap,JobScheduler *scheduler) {

    Parameter *threadParameter = new Parameter(head,heap,scheduler);

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

Parameter::Parameter(Head *newhead,MaxHeap *newheap,JobScheduler *newscheduler){
    head = newhead;
    heap = newheap;
    scheduler = newscheduler;
}
