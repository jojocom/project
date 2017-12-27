pthread_mutex_t mtx;
pthread_cond_t cond_nonempty;
pthread_cond_t cond_nonfull;
Queue * queue;


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
    // for (int i = 0; i < count; i++) {
    //     delete data[start+i];
    // }
}


JobScheduler::JobScheduler( int newexecution_threads):execution_threads(newexecution_threads) {
    queue = new Queue();
    if((tids = (pthred_t*) malloc(newexecution_threads * sizeof(pthred_t))) == NULL) {
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

void JobScheduler::execute_all_jobs() {
    for (int i = 0; i < execution_threads; i++) {
        if((err = pthread_create(tids+i,NULL,execute))) {
            perror2("pthread_create",err);
            exit(1);
        }
    }
}

void JobScheduler::wait_all_tasks_finish() { //waits all submitted tasks to finish
    for (i = 0; i < execution_threads; i++) {
        if ((err = pthread_join(*(tids + i), NULL))) { // Wait for thread termination
            perror2("pthread_join", err);
            exit(1);
        }

}

void execute(){

}

// Job * obtain() {
//     Job * data = NULL;
//     pthread_mutex_lock(&mtx);
//     while (queue->count <= 0) {
//         pthread_cond_wait(&cond_nonempty, &mtx);
//     }
//     data = queue->data[queue->start];
//     queue->start = (queue->start + 1) % POOL_SIZE;
//     queue->count--;
//     pthread_cond_broadcast(&cond_nonfull);
//     pthread_mutex_unlock(&mtx);
//     return data;
// }
