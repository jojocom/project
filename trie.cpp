#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <stdint.h>
#include <unistd.h>
#include "list.h"
#include "hash_functions.h"
#include "jobScheduler.h"
#define STARTSIZE 10
#define THREADSNUM 8

using namespace std;

// constructor of trie nodes
RootNode::RootNode() : split(0) , size(HashSize), newsize(2*HashSize), currentSize(HashSize) {
    hashtable = (TrieNode ***) malloc(sizeof(TrieNode **)*HashSize);
    for (int i = 0; i < HashSize; i++) {
        hashtable[i] = (TrieNode **) malloc(sizeof(TrieNode *));
    }
    for (int i = 0; i < HashSize; i++) {
        *hashtable[i] = (TrieNode *) malloc(sizeof(TrieNode)*BucketLen);
        for (int j = 0; j < BucketLen; j++) {
            setNode(&(*hashtable[i])[j],N,false,NULL);
        }
    }
    bucketSizes = (int *) malloc(sizeof(int)*HashSize);
    for (int i = 0; i < HashSize; i++) {
        bucketSizes[i] = BucketLen;
    }
    items = (int *) malloc(sizeof(int)*HashSize);
    for (int i = 0; i < HashSize; i++) {
        items[i] = 0;
    }
}

// destructor of trie nodes
RootNode::~RootNode(){
    for (int i = 0; i < currentSize; i++) {
        for(int j = 0; j < bucketSizes[i]; j++){
            if((*hashtable[i])[j].childs != NULL){
                if(*(*hashtable[i])[j].childs != NULL){
                    for (int k = 0; k < (*hashtable[i])[j].capacity; k++) {
                        free_trie(&(*(*hashtable[i])[j].childs)[k]);
                        if((*(*hashtable[i])[j].childs)[k].word != NULL){
                            free((*(*hashtable[i])[j].childs)[k].word);
                        }
                        if((*(*hashtable[i])[j].childs)[k].compressedLengths != NULL){
                            free((*(*hashtable[i])[j].childs)[k].compressedLengths);
                        }
                    }
                    free(*(*hashtable[i])[j].childs);
                    free((*hashtable[i])[j].childs);
                }
            }
            if((*hashtable[i])[j].word != NULL){
                free((*hashtable[i])[j].word);
            }
            if((*hashtable[i])[j].compressedLengths != NULL){
                free((*hashtable[i])[j].compressedLengths);
            }
        }
        free(*hashtable[i]);
        free(hashtable[i]);
    }
    free(hashtable);
    free(bucketSizes);
    free(items);
}

// constructor of trie nodes
TrieNode::TrieNode(int capacity) : capacity(capacity), childNum(0), final(false),compressedNum(0){
    word = NULL;
    compressedLengths = NULL;
    childs = (TrieNode **) malloc(sizeof(TrieNode *));
    *childs = (TrieNode *) malloc(sizeof(TrieNode)*capacity);
    for (int i = 0; i < capacity; i++) {
        setNode(&(*childs)[i],N,false,NULL);
    }
}

// destructor of trie nodes
TrieNode::~TrieNode(){
    if(childs != NULL){
        if(*childs != NULL){
            for (int i = 0; i < capacity; i++) {
                free_trie(&(*childs)[i]);
                if((*childs)[i].word != NULL){
                    free((*childs)[i].word);
                }
                if((*childs)[i].compressedLengths != NULL){
                    free((*childs)[i].compressedLengths);
                }
            }
            free(*childs);
            free(childs);
        }
    }
    if(word != NULL){
        free(word);
    }
    if(compressedLengths != NULL){
        free(compressedLengths);
    }
}

// set values of trie node members
void setNode(TrieNode *current,int capacity,bool final,char *word){
    current->capacity = capacity;
    current->childNum = 0;
    current->final = final;
    if(word != NULL){
        current->word = (char *) malloc(sizeof(char)*(strlen(word) + 1));
        strcpy(current->word,word);
    } else {
        current->word = NULL;
    }
    current->childs = NULL;
    current->compressedNum = 0;
    current->compressedLengths = NULL;
}

// constructor of head
Head::Head(){
    root = new RootNode();
}

// destructor of head
Head::~Head(){
    delete root;
}

// recursive delete of trie
void free_trie(TrieNode *current){

    if(current->childs == NULL){            // if node has no childs
        return;
    }

    for (int i = 0; i < current->capacity; i++){        // delete each child
        free_trie(&(*current->childs)[i]);
        if ((*current->childs)[i].word != NULL) {
            free((*current->childs)[i].word);
        }
        if ((*current->childs)[i].compressedLengths != NULL) {
            free((*current->childs)[i].compressedLengths);
        }
    }
    free(*current->childs);             // delete array of childs
    free(current->childs);              // delete pointer to array of childs
}

// initializing of trie with Ngrams
Head * trieCreate(char *fileName, int *compress){

    Head *head = new Head();
    string line;
    ifstream queryFile( fileName );
    if (queryFile) {
        while (getline( queryFile, line )) {        // read Ngram
            char *cline = (char *) malloc(sizeof(char)*(line.length() + 1));
            strcpy(cline, line.c_str());
            if(strcmp(cline,"DYNAMIC") == 0){
                *compress = 0;
                free(cline);
                continue;
            } else if(strcmp(cline,"STATIC") == 0){
                *compress = 1;
                free(cline);
                continue;
            }
            int whitespace = 0;
            for(int i=0 ; i<(int) strlen(cline) ; i++){     //counting spaces
                if(i != (int) strlen(cline) - 1){
                    if((cline[i] == ' ') && (cline[i+1] != ' ')) {
                        whitespace++;
                    }
                }
            }

            char *temp;
            char *position;
            char **query;
            query = (char **) malloc(sizeof(char *)*(whitespace + 1));
            temp = cline;
            for (int i = 0; i <= whitespace; i++) {     // creating array of strings of the Ngram
                position = strchr(temp,' ');
                if((temp[0] == ' ')) {
                    temp = position + 1;
                    i--;
                    continue;
                }
                if(i != whitespace){
                    query[i] = (char *) malloc(sizeof(char)*(position - temp + 1));
                } else {
                    query[i] = (char *) malloc(sizeof(char)*(strlen(temp) + 1));
                }
                sscanf(temp,"%s",query[i]);

                temp = position + 1;
            }
            insertNgram(query,whitespace+1,head);       // inserting Ngram to the trie
            for(int i = 0; i <= whitespace; i++){
                free(query[i]);
            }
            free(query);
            free(cline);
        }
        queryFile.close();
    }

    return head;
}

// inserting Ngram to the trie
void insertNgram(char **query,int queryNum,Head *head){
    TrieNode *currentNode = NULL;
    int found = 0;
    int hash = hashcii(query[0]) % head->root->size;
    if(hash < head->root->split){
        hash = hashcii(query[0]) % head->root->newsize;
    }
    int left = 0;
    int right = head->root->items[hash] - 1;
    while(left <= right){                       // checking if 1st word of Ngram exists and where
        int mid = left + ((right - left)/2);
        if(head->root->hashtable[hash] != NULL){
            if((*head->root->hashtable[hash])[mid].word != NULL){
                if(strcmp(query[0],(*head->root->hashtable[hash])[mid].word) == 0){
                    found = 1;
                    currentNode = &(*head->root->hashtable[hash])[mid];
                    break;
                } else if(strcmp(query[0],(*head->root->hashtable[hash])[mid].word) < 0){
                    right = mid - 1;
                } else if(strcmp(query[0],(*head->root->hashtable[hash])[mid].word) > 0){
                    left = mid + 1;
                }
            }
        }
    }
    if(found == 0){
        if(head->root->bucketSizes[hash] == head->root->items[hash]){
            *head->root->hashtable[hash] = (TrieNode *) realloc((*head->root->hashtable[hash]),sizeof(TrieNode)*2*(head->root->bucketSizes[hash]));
            head->root->bucketSizes[hash] = head->root->bucketSizes[hash]*2;
            for (int i = head->root->bucketSizes[hash]/2; i < head->root->bucketSizes[hash]; i++) {
                setNode(&(*head->root->hashtable[hash])[i],N,false,NULL);
            }
            if(queryNum == 1){
                setNode(&(*head->root->hashtable[hash])[head->root->items[hash]],N,true,query[0]);
            } else{
                setNode(&(*head->root->hashtable[hash])[head->root->items[hash]],N,false,query[0]);
            }
            (*head->root->hashtable[hash])[head->root->items[hash]].childs = (TrieNode **) malloc(sizeof(TrieNode*));
            *(*head->root->hashtable[hash])[head->root->items[hash]].childs = (TrieNode *) malloc(sizeof(TrieNode)*N);
            for (int j = 0; j < N; j++) {
                setNode(&(*(*head->root->hashtable[hash])[head->root->items[hash]].childs)[j],N,false,NULL);
            }
            head->root->items[hash]++;
            if(hash != head->root->split){
                int cell = head->root->items[hash] - 1;
                if(head->root->items[hash] > 1){
                    cell = sortCell(head->root,hash);
                }
                currentNode = &(*head->root->hashtable[hash])[cell];
            }
            head->root->hashtable = (TrieNode ***) realloc(head->root->hashtable,sizeof(TrieNode **)*(head->root->currentSize+1));
            head->root->currentSize++;
            head->root->hashtable[head->root->currentSize-1] = (TrieNode **) malloc(sizeof(TrieNode *));
            *head->root->hashtable[head->root->currentSize-1] = (TrieNode *) malloc(sizeof(TrieNode)*BucketLen);
            for (int e = 0; e < BucketLen; e++) {
                setNode(&(*head->root->hashtable[head->root->currentSize-1])[e],N,false,NULL);
            }
            head->root->items = (int *) realloc(head->root->items,sizeof(int)*head->root->currentSize);
            head->root->items[head->root->currentSize-1] = 0;
            head->root->bucketSizes = (int *) realloc(head->root->bucketSizes,sizeof(int)*head->root->currentSize);
            head->root->bucketSizes[head->root->currentSize-1] = BucketLen;
            // split bucket
            for (int i = 0; i < head->root->items[head->root->split]; i++) {
                int newhash = 0;
                newhash = hashcii((*head->root->hashtable[head->root->split])[i].word) % head->root->newsize;
                if(newhash != head->root->split){
                    if(head->root->bucketSizes[newhash] == head->root->items[newhash]){
                        *head->root->hashtable[newhash] = (TrieNode *) realloc((*head->root->hashtable[newhash]),sizeof(TrieNode)*2*(head->root->bucketSizes[newhash]));
                        head->root->bucketSizes[newhash] = head->root->bucketSizes[newhash]*2;
                        for (int i = head->root->bucketSizes[newhash]/2; i < head->root->bucketSizes[newhash]; i++) {
                            setNode(&(*head->root->hashtable[newhash])[i],N,false,NULL);
                        }
                    }
                    // move to new bucket
                    memmove(&(*head->root->hashtable[newhash])[head->root->items[newhash]],&(*head->root->hashtable[head->root->split])[i],sizeof(TrieNode));
                    if((head->root->items[head->root->split]-i-1) != 0){
                        // move other items 1 position left
                        memmove(&(*head->root->hashtable[head->root->split])[i],&(*head->root->hashtable[head->root->split])[i+1],sizeof(TrieNode)*(head->root->items[head->root->split]-i-1));
                    }
                    setNode(&(*head->root->hashtable[head->root->split])[head->root->items[head->root->split]-1],N,false,NULL);
                    head->root->items[newhash]++;
                    head->root->items[head->root->split]--;
                    i--;
                }
                if((i == head->root->items[head->root->split]-1) && (head->root->split == hash)){
                    int cell = head->root->items[newhash] - 1;
                    if(head->root->items[newhash] > 1){
                        cell = sortCell(head->root,newhash);
                    }
                    currentNode = &(*head->root->hashtable[newhash])[cell];
                }
            }
            head->root->split++;
            if(head->root->split == head->root->size){
                head->root->split = 0;
                head->root->size = head->root->newsize;
                head->root->newsize = 2*head->root->newsize;
            }
        } else{
            if(queryNum == 1){
                setNode(&(*head->root->hashtable[hash])[head->root->items[hash]],N,true,query[0]);
            } else{
                setNode(&(*head->root->hashtable[hash])[head->root->items[hash]],N,false,query[0]);
            }
            (*head->root->hashtable[hash])[head->root->items[hash]].childs = (TrieNode **) malloc(sizeof(TrieNode*));
            *(*head->root->hashtable[hash])[head->root->items[hash]].childs = (TrieNode *) malloc(sizeof(TrieNode)*N);
            for (int j = 0; j < N; j++) {
                setNode(&(*(*head->root->hashtable[hash])[head->root->items[hash]].childs)[j],N,false,NULL);
            }
            head->root->items[hash]++;
            int cell = head->root->items[hash] - 1;
            if(head->root->items[hash] > 1){
                cell = sortCell(head->root,hash);
            }
            currentNode = &(*head->root->hashtable[hash])[cell];
        }
    } else{
        if(queryNum == 1){
            currentNode->final = true;
        }
    }
    for (int i = 1; i < queryNum; i++) {                // for each word of the Ngram
        found = 0;
        left = 0;
        right = currentNode->childNum - 1;
        while(left <= right){                       // checking if i-th word of Ngram exists and where
            int mid = left + ((right - left)/2);
            if(currentNode->childs != NULL){
                if((*currentNode->childs)[mid].word != NULL){
                    if(strcmp(query[i],(*currentNode->childs)[mid].word) == 0){
                        found = 1;
                        currentNode = &(*currentNode->childs)[mid];
                        if(i == queryNum - 1){          // if all Ngram words exists, checking if last word of Ngram is final
                            currentNode->final = true;
                        }
                        break;
                    } else if(strcmp(query[i],(*currentNode->childs)[mid].word) < 0){
                        right = mid - 1;
                    } else if(strcmp(query[i],(*currentNode->childs)[mid].word) > 0){
                        left = mid + 1;
                    }
                }
            }
        }
        if(found == 0){                     // if i-th word of Ngram does not exist
            if(currentNode->childNum == currentNode->capacity){             // if array of childs need expansion
                *currentNode->childs = (TrieNode *) realloc(*currentNode->childs,sizeof(TrieNode)*currentNode->capacity*2);
                currentNode->capacity = currentNode->capacity*2;
                for (int j = currentNode->childNum; j < currentNode->capacity; j++) {
                    setNode(&(*currentNode->childs)[j],N,false,NULL);
                }
            }
            if(i == queryNum - 1){                  // creating node for last word of Ngram
                setNode(&(*currentNode->childs)[currentNode->childNum],N,true,query[i]);
                (*currentNode->childs)[currentNode->childNum].childs = (TrieNode **) malloc(sizeof(TrieNode*));
                *(*currentNode->childs)[currentNode->childNum].childs = (TrieNode *) malloc(sizeof(TrieNode)*N);
                for (int j = 0; j < N; j++) {
                    setNode(&(*(*currentNode->childs)[currentNode->childNum].childs)[j],N,false,NULL);
                }
            } else {                        // creating node for not last word of Ngram
                setNode(&(*currentNode->childs)[currentNode->childNum],N,false,query[i]);
                (*currentNode->childs)[currentNode->childNum].childs = (TrieNode **) malloc(sizeof(TrieNode*));
                *(*currentNode->childs)[currentNode->childNum].childs = (TrieNode *) malloc(sizeof(TrieNode)*N);
                for (int j = 0; j < N; j++) {
                    setNode(&(*(*currentNode->childs)[currentNode->childNum].childs)[j],N,false,NULL);
                }
            }
            currentNode->childNum++;

            int cell = currentNode->childNum - 1;
            if(currentNode->childNum > 1){
                cell = sortChilds(currentNode);             // binary insertion sort of the array of childs
            }
            currentNode = &(*currentNode->childs)[cell];
        }
    }
}

// searching Ngram int the trie
int searchNgram(char **query,int queryNum,Head *head){

    TrieNode *currentNode = NULL;
    int found = 0;
    int hash = hashcii(query[0]) % head->root->size;
    if(hash < head->root->split){
        hash = hashcii(query[0]) % head->root->newsize;
    }
    int left = 0;
    int right = head->root->items[hash] - 1;
    while(left <= right){                       // checking if 1st word of Ngram exists and where
        int mid = left + ((right - left)/2);
        if(head->root->hashtable[hash] != NULL){
            if((*head->root->hashtable[hash])[mid].word != NULL){
                if(strcmp(query[0],(*head->root->hashtable[hash])[mid].word) == 0){
                    found = 1;
                    currentNode = &(*head->root->hashtable[hash])[mid];
                    if(queryNum == 1){          // if all Ngram words exists, checking if last word of Ngram is final
                        if(currentNode->final == false){
                            found = 2;
                        }
                    }
                    break;
                } else if(strcmp(query[0],(*head->root->hashtable[hash])[mid].word) < 0){
                    right = mid - 1;
                } else if(strcmp(query[0],(*head->root->hashtable[hash])[mid].word) > 0){
                    left = mid + 1;
                }
            }
        }
    }
    if(found == 1){
        for (int i = 1; i < queryNum; i++) {                // for each word of the Ngram
            found = 0;
            int left = 0;
            int right = currentNode->childNum - 1;
            while(left <= right){                       // checking if i-th word of Ngram exists and where
                int mid = left + ((right - left)/2);
                if(currentNode->childs != NULL){
                    if((*currentNode->childs)[mid].word != NULL){
                        if(strcmp(query[i],(*currentNode->childs)[mid].word) == 0){
                            found = 1;
                            currentNode = &(*currentNode->childs)[mid];
                            if(i == queryNum - 1){          // if all Ngram words exists, checking if last word of Ngram is final
                                if(currentNode->final == false){
                                    found = 2;
                                }
                            }
                            break;
                        } else if(strcmp(query[i],(*currentNode->childs)[mid].word) < 0){
                            right = mid - 1;
                        } else if(strcmp(query[i],(*currentNode->childs)[mid].word) > 0){
                            left = mid + 1;
                        }
                    }
                }
            }
            if(found == 0){                     // if i-th word of Ngram does not exist
                return 0;
            }
        }
    }
    if(found == 1){
        return 1;
    } else if(found == 2){
        return 2;
    }

    return 0;
}

// deleting Ngram from the trie
void deleteNgram(char **query,int queryNum,Head *head){

    TrieNode **path = new TrieNode*[queryNum+1];
    TrieNode *currentNode = NULL;
    int found = 0;
    int hash = hashcii(query[0]) % head->root->size;
    if(hash < head->root->split){
        hash = hashcii(query[0]) % head->root->newsize;
    }
    int mid = 0;
    int left = 0;
    int right = head->root->items[hash] - 1;
    while(left <= right){                       // checking if 1st word of Ngram exists and where
        mid = left + ((right - left)/2);
        if(head->root->hashtable[hash] != NULL){
            if((*head->root->hashtable[hash])[mid].word != NULL){
                if(strcmp(query[0],(*head->root->hashtable[hash])[mid].word) == 0){
                    found = 1;
                    currentNode = &(*head->root->hashtable[hash])[mid];
                    break;
                } else if(strcmp(query[0],(*head->root->hashtable[hash])[mid].word) < 0){
                    right = mid - 1;
                } else if(strcmp(query[0],(*head->root->hashtable[hash])[mid].word) > 0){
                    left = mid + 1;
                }
            }
        }
    }
    if(found == 1){
        if((currentNode->childNum == 0) && (queryNum == 1)){
            free_trie(currentNode);             //delete node
            if(currentNode->word != NULL){
                free(currentNode->word);
            }
            memmove(&(*head->root->hashtable[hash])[mid],&(*head->root->hashtable[hash])[mid+1],sizeof(TrieNode)*(head->root->items[hash]-mid-1));
            setNode(&(*head->root->hashtable[hash])[head->root->items[hash]-1],N,false,NULL);
            head->root->items[hash]--;
        } else{
            path[0] = currentNode;
            for (int i = 1; i < queryNum; i++) {                // for each word of the Ngram
                found = 0;
                int left = 0;
                int right = currentNode->childNum - 1;
                while(left <= right){                       // checking if i-th word of Ngram exists and where
                    int mid = left + ((right - left)/2);
                    if(currentNode->childs != NULL){
                        if((*currentNode->childs)[mid].word != NULL){
                            if(strcmp(query[i],(*currentNode->childs)[mid].word) == 0){
                                found = 1;
                                currentNode = &(*currentNode->childs)[mid];
                                path[i] = currentNode;
                                if(i == queryNum - 1){          // if all Ngram words exists, checking if last word of Ngram is final
                                    if(currentNode->final == false){
                                        found = 0;
                                    }
                                }
                                break;
                            } else if(strcmp(query[i],(*currentNode->childs)[mid].word) < 0){
                                right = mid - 1;
                            } else if(strcmp(query[i],(*currentNode->childs)[mid].word) > 0){
                                left = mid + 1;
                            }
                        }
                    }
                }
                if(found == 0){                     // if i-th word of Ngram does not exist
                    for (int j = 0; j < queryNum; j++) {
                        path[j] = NULL;
                    }
                    delete [] path;
                    return ;
                }
            }
            for (int i = queryNum; i > 0; i--) {                // deleting node of i-th word of Ngram
                if(i == queryNum){                      // if node of last word of Ngram
                    if(path[i-1]->childNum != 0){           // if node has childs
                        path[i-1]->final = false;
                        break;
                    } else{
                        int cell = 0;
                        for (int j = 0; j < path[i-2]->childNum; j++) {         // find position of node in parent's array of childs
                            if(strcmp((*path[i-2]->childs)[j].word,path[i-1]->word) == 0){
                                cell = j;
                                break;
                            }
                        }
                        free_trie(&(*path[i-2]->childs)[cell]);             //delete node
                        if((*path[i-2]->childs)[cell].word != NULL){
                            free((*path[i-2]->childs)[cell].word);
                        }
                        path[i-2]->childNum--;
                        memmove(&(*path[i-2]->childs)[cell],&(*path[i-2]->childs)[cell+1],sizeof(TrieNode)*(path[i-2]->childNum-cell));         // move childs that are right of the node, that we deleted, in the array
                        setNode(&(*path[i-2]->childs)[path[i-2]->childNum],N,false,NULL);
                    }
                } else if(i == 1){                      // if node of not last word of Ngram
                    if((path[i-1]->childNum == 0) && (path[i-1]->final == false)){
                        free_trie(&(*head->root->hashtable[hash])[mid]);             //delete node
                        if((*head->root->hashtable[hash])[mid].word != NULL){
                            free((*head->root->hashtable[hash])[mid].word);
                        }
                        memmove(&(*head->root->hashtable[hash])[mid],&(*head->root->hashtable[hash])[mid+1],sizeof(TrieNode)*(head->root->items[hash]-mid-1));
                        setNode(&(*head->root->hashtable[hash])[head->root->items[hash]-1],N,false,NULL);
                        head->root->items[hash]--;
                    } else break;
                } else{                      // if node of not last word of Ngram
                    if((path[i-1]->childNum == 0) && (path[i-1]->final == false) && (path[i-1]->word != NULL)){           // if node has not childs,is not final and is not head->root
                        int cell = 0;
                        for (int j = 0; j < path[i-2]->childNum; j++) {         // find position of node in parent's array of childs
                            if(strcmp((*path[i-2]->childs)[j].word,path[i-1]->word) == 0){
                                cell = j;
                                break;
                            }
                        }
                        free_trie(&(*path[i-2]->childs)[cell]);             //delete node
                        if((*path[i-2]->childs)[cell].word != NULL){
                            free((*path[i-2]->childs)[cell].word);
                        }
                        path[i-2]->childNum--;
                        memmove(&(*path[i-2]->childs)[cell],&(*path[i-2]->childs)[cell+1],sizeof(TrieNode)*(path[i-2]->childNum-cell));         // move childs that are right of the node, that we deleted, in the array
                        setNode(&(*path[i-2]->childs)[path[i-2]->childNum],N,false,NULL);
                    } else break;
                }
            }
        }
    }
    for (int j = 0; j < queryNum; j++) {
        path[j] = NULL;
    }
    delete [] path;
    return ;
}

// reading queries and do Q,A or D
void queryRead(char *queryFileName,Head *head){

    MaxHeap *heap = new MaxHeap(HeapCap);    // heap create

    string line;
    ifstream queryFile( queryFileName );
    if (queryFile) {
        while (getline( queryFile, line )) {                // reading queries
            char *cline = (char *) malloc(sizeof(char)*(line.length() + 1));
            strcpy(cline, line.c_str());
            int whitespace = 0;
            for(int i=0 ; i<(int) strlen(cline) ; i++){         // counting spaces
                if(i != (int) strlen(cline) - 1){
                    if((cline[i] == ' ') && (cline[i+1] != ' ')) {
                        whitespace++;
                    }
                }
            }
            char *temp;
            char *position;
            char **query;
            query = (char **) malloc(sizeof(char *)*(whitespace + 1));
            temp = cline;
            for (int i = 0; i <= whitespace; i++) {         // creating array of strings of the words of the query
                position = strchr(temp,' ');
                if((temp[0] == ' ')) {
                    temp = position + 1;
                    i--;
                    continue;
                }
                if(i != whitespace){
                    query[i] = (char *) malloc(sizeof(char)*(position - temp + 1));
                } else {
                    query[i] = (char *) malloc(sizeof(char)*(strlen(temp) + 1));
                }
                sscanf(temp,"%s",query[i]);
                temp = position + 1;
            }
            char **queryStart = query + 1;
            uint32_t bitArray[M];
            for (int i = 0; i < M; i++) {
                bitArray[i] = 0;
            }
            if(strcmp(query[0],"Q") == 0){              // for Q query
                int found = 0;
                for (int k = 0; k < whitespace; k++) {
                    char **temp = queryStart + k;
                    for (int j = 1; j <= whitespace-k; j++) {
                        int x = searchNgram(temp,j,head);
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
                                    // this is hell
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
                                    cout << mykey;
                                } else{
                                    cout << "|";
                                    cout << mykey;
                                }
                                heap->insertKey(mykey);
                            }
                            free(mykey);
                            found = 1;
                        } else if(x == 0){
                            break;
                        }
                    }
                }
                if(found == 0){
                    cout << "-1";               // if there are no Ngrams int the query
                }
                cout << endl;
            }
            else if (strcmp(query[0],"A") == 0){              // for A query
                insertNgram(queryStart,whitespace,head);
            }
            else if (strcmp(query[0],"D") == 0){              // for D query
                deleteNgram(queryStart,whitespace,head);
            }
            else if (strcmp(query[0],"F") == 0){
                if(whitespace != 0){
                    int k = atoi(query[1]);
                    if(heap->elements != 0){
                        cout << "Top: ";
                        for (int i = 0; i < k; i++) {
                            Element *el1 = heap->extractMax();
                            if(el1 != NULL){
                                if( i == k-1){
                                    cout << el1->word;
                                } else{
                                    cout << el1->word << "|";
                                }
                                if(heap->elements != 0){
                                    delete el1;
                                }
                            }
                        }
                        cout << endl;
                    }
                    delete heap;
                    heap = new MaxHeap(HeapCap);
                } else{
                    delete heap;
                    heap = new MaxHeap(HeapCap);
                }
            }
            else if (strcmp(query[0],"P") == 0){
                // printAll(head->root,0);
            }

            for(int i = 0; i <= whitespace; i++){
                free(query[i]);
            }
            free(query);
            free(cline);
        }
        queryFile.close();
    }
    delete heap;
}

void print(RootNode *root){
    cout << "hashtable" << endl;
    for (int i = 0; i < root->currentSize; i++) {
        cout << "\tbucket: " << i << endl;
        for (int j = 0; j < root->items[i]; j++) {
            cout << "\t\titem: " << j << endl;
            printAll(&(*root->hashtable[i])[j],3);
        }
    }
}

// printing of trie
void printAll(TrieNode *root,int tabs){
    if(root->word != NULL){
        for (int i = 0; i < tabs; i++) {
            cout << "\t";
        }
        cout << root->word << " " << root->final << " " << root->capacity << " " << root->childNum << " ";
        if(root->compressedLengths != NULL){
            cout << "(";
            for (int i = 0; i < root->compressedNum; i++) {
                if(i == root->compressedNum - 1){
                    cout << root->compressedLengths[i];
                } else{
                    cout << root->compressedLengths[i] << ",";
                }
            }
            cout << ")" << endl;
        } else {
            cout << "()" << endl;
        }
    } else {
        cout << root->final << " " << root->capacity << " " << root->childNum << endl;
    }
    for(int looper = 0;looper < root->childNum;looper++){
        if (&(*root->childs)[looper] != NULL) {
            printAll(&(*root->childs)[looper],tabs+1);
        }
    }
}

// binary insertion sort to an array of trie nodes
int sortChilds(TrieNode *currentNode){

    int left = 0;
    int right = currentNode->childNum-2;
    while(left <= right){
        int mid = left + ((right - left)/2);
        if(strcmp((*currentNode->childs)[currentNode->childNum-1].word,(*currentNode->childs)[mid].word) < 0){
            right = mid - 1;
        } else if(strcmp((*currentNode->childs)[currentNode->childNum-1].word,(*currentNode->childs)[mid].word) > 0){
            left = mid + 1;
        }
    }
    // moving inserted node to the right position in the array
    TrieNode *tempNode = (TrieNode *) malloc(sizeof(TrieNode)*(currentNode->childNum-1-left));
    memcpy(tempNode,&(*currentNode->childs)[left],sizeof(TrieNode)*(currentNode->childNum-1-left));
    memmove(&(*currentNode->childs)[left],&(*currentNode->childs)[currentNode->childNum-1],sizeof(TrieNode));
    memcpy(&(*currentNode->childs)[left+1],tempNode,sizeof(TrieNode)*(currentNode->childNum-1-left));
    free(tempNode);

    return left;
}

// binary insertion sort to an array of trie nodes
int sortCell(RootNode *root,int hash){
    int left = 0;
    int right = root->items[hash]-2;
    while(left <= right){
        int mid = left + ((right - left)/2);
        if(strcmp((*root->hashtable[hash])[root->items[hash]-1].word,(*root->hashtable[hash])[mid].word) < 0){
            right = mid - 1;
        } else if(strcmp((*root->hashtable[hash])[root->items[hash]-1].word,(*root->hashtable[hash])[mid].word) > 0){
            left = mid + 1;
        }
    }
    // moving inserted node to the right position in the array
    TrieNode *tempNode = (TrieNode *) malloc(sizeof(TrieNode)*(root->items[hash]-1-left));
    memcpy(tempNode,&(*root->hashtable[hash])[left],sizeof(TrieNode)*(root->items[hash]-1-left));
    memmove(&(*root->hashtable[hash])[left],&(*root->hashtable[hash])[root->items[hash]-1],sizeof(TrieNode));
    memcpy(&(*root->hashtable[hash])[left+1],tempNode,sizeof(TrieNode)*(root->items[hash]-1-left));
    free(tempNode);

    return left;
}

void trieCompress(Head *head){
    for (int i = 0; i < head->root->currentSize; i++) {
        for(int j = 0; j < head->root->items[i]; j++){
            recursiveCompress(&(*head->root->hashtable[i])[j]);
        }
    }
}

void recursiveCompress(TrieNode *current){
    if(current->childNum == 0){
        current->compressedNum = 1;
        current->compressedLengths = (short *) malloc(sizeof(short));
        current->compressedLengths[0] = strlen(current->word);
        return;
    } else if(current->childNum == 1){
        recursiveCompress(&(*current->childs)[0]);
        int len = strlen(current->word);
        if(current->final == 0){
            len = len*(-1);
        }
        current->compressedNum = (*current->childs)[0].compressedNum + 1;
        current->compressedLengths = (short *) malloc(sizeof(short)*current->compressedNum);
        current->compressedLengths[0] = len;
        for (int i = 1; i < current->compressedNum; i++) {
            current->compressedLengths[i] = (*current->childs)[0].compressedLengths[i-1];
        }
        current->word = (char *) realloc(current->word,sizeof(char)*(strlen(current->word)+strlen((*current->childs)[0].word)+1));
        strcat(current->word,(*current->childs)[0].word);
        current->childNum = (*current->childs)[0].childNum;
        TrieNode **tempChilds = (*current->childs)[0].childs;
        (*current->childs)[0].childs = NULL;
        free_trie(current);
        current->childs = tempChilds;
        return;
    } else{
        current->compressedNum = 1;
        current->compressedLengths = (short *) malloc(sizeof(short));
        int len = strlen(current->word);
        if(current->final == 0){
            len = len*(-1);
        }
        current->compressedLengths[0] = len;
        for (int i = 0; i < current->childNum; i++) {
            recursiveCompress(&(*current->childs)[i]);
        }
        return;
    }
}

void queryStaticRead(char *queryFileName,Head *head){

    // cout << "1" << endl;

    MaxHeap *heap = new MaxHeap(HeapCap);    // heap create

    int counter = 0;
    int currentSize = STARTSIZE;

    Job **jobsArray = (Job **) malloc(sizeof(Job *)*STARTSIZE);
    for (int i = 0; i < STARTSIZE; i++) {
        jobsArray[i] = NULL;
    }

    JobScheduler *scheduler = new JobScheduler(THREADSNUM);

    pthread_mutex_init(&queue_mtx, 0);
    pthread_mutex_init(&heap_mtx, 0);
    pthread_mutex_init(&printer_mtx, 0);
    pthread_cond_init(&cond_nonempty, 0);
    pthread_cond_init(&cond_nonfull, 0);

    threadParameter = new Parameter(head,heap,scheduler);

    // sleep(2);

    string line;
    ifstream queryFile( queryFileName );
    if (queryFile) {
        while (getline( queryFile, line )) {                // reading queries
            // cout << "2" << endl;
            char *cline = (char *) malloc(sizeof(char)*(line.length() + 1));
            strcpy(cline, line.c_str());
            int whitespace = 0;
            for(int i=0 ; i<(int) strlen(cline) ; i++){         // counting spaces
                if(i != (int) strlen(cline) - 1){
                    if((cline[i] == ' ') && (cline[i+1] != ' ')) {
                        whitespace++;
                    }
                }
            }
            char *temp;
            char *position;
            char **query;
            query = (char **) malloc(sizeof(char *)*(whitespace + 1));
            temp = cline;
            for (int i = 0; i <= whitespace; i++) {         // creating array of strings of the words of the query
                position = strchr(temp,' ');
                if((temp[0] == ' ')) {
                    temp = position + 1;
                    i--;
                    continue;
                }
                if(i != whitespace){
                    query[i] = (char *) malloc(sizeof(char)*(position - temp + 1));
                } else {
                    query[i] = (char *) malloc(sizeof(char)*(strlen(temp) + 1));
                }
                sscanf(temp,"%s",query[i]);
                temp = position + 1;
            }

            if(counter == currentSize){
                jobsArray = (Job **) realloc(jobsArray,sizeof(Job *)*currentSize*2);
                for (int i = currentSize; i < 2*currentSize; i++) {
                    jobsArray[i] = NULL;
                }
                currentSize = 2*currentSize;
            }
            jobsArray[counter] = new Job(counter,whitespace+1,query);
            // cout << "id: " << jobsArray[counter]->id << " queryLen: " << jobsArray[counter]->queryLen << endl;
            // for (int i = 0; i < jobsArray[counter]->queryLen; i++) {
            //     cout << jobsArray[counter]->query[i] << " ";
            // }
            // cout << endl;

            counter++;
            if (strcmp(query[0],"F") == 0){
                // cout << "3" << endl;
                // create threads
                char **printer = (char **) malloc(sizeof(char *)*(counter-1));
                threadParameter->printer = printer;

                scheduler->execute_all_jobs(1);


                for (int i = 0; i < counter-1; i++) {
                    // submit jobs
                    // cout << "3a" << endl;
                    // cout << "id: " << jobsArray[counter]->id << " queryLen: " << jobsArray[counter]->queryLen << endl;
                    // for (int i = 0; i < jobsArray[counter]->queryLen; i++) {
                    //     cout << jobsArray[counter]->query[i] << " ";
                    // }
                    // cout << endl;
                    scheduler->submit_job(jobsArray[i]);
                    // cout << "id: " << scheduler->queue->data[scheduler->queue->end]->id << " queryLen: " << scheduler->queue->data[scheduler->queue->end]->queryLen << endl;
                    // for (int j = 0; j < scheduler->queue->data[scheduler->queue->end]->queryLen; j++) {
                    //     cout << scheduler->queue->data[scheduler->queue->end]->query[j] << " ";
                    // }
                    // cout << endl;
                    // cout << "3b" << endl;
                }

                // poisoning threads
                char **poison = (char **) malloc(sizeof(char*));
                poison[0] = (char *) malloc(sizeof(char)*(strlen("poison")+1));
                strcpy(poison[0],"poison");
                // cout << poison[0] << endl;
                // cout << 111 << endl;
                Job *poisonJob = new Job(-1,1,poison);
                for (int i = 0; i < THREADSNUM; i++) {
                    // submit poison jobs
                    scheduler->submit_job(poisonJob);
                }

                // wait threads
                int err;
                for (int i = 0; i < THREADSNUM; i++) {
                    if ((err = pthread_join(*(scheduler->tids + i), NULL))) { // Wait for thread termination
                        perror("pthread_join");
                        exit(1);
                    }
                }

                for (int i = 0; i < counter-1; i++) {
                    cout << printer[i] << endl;
                }

                for (int i = 0; i < counter-1; i++) {
                    free(printer[i]);
                }
                free(printer);
                threadParameter->printer = NULL;

                if(whitespace != 0){
                    int k = atoi(query[1]);
                    cout << "Top: ";
                    for (int i = 0; i < k; i++) {
                        pthread_mutex_lock(&heap_mtx);
                        Element *el1 = heap->extractMax();
                        pthread_mutex_unlock(&heap_mtx);
                        if(el1 != NULL){
                            if( i == k-1){
                                cout << el1->word << endl;
                            } else{
                                cout << el1->word << "|";
                            }
                            if(heap->elements != 0){
                                delete el1;
                            }
                        }
                    }
                }

                delete heap;
                heap = new MaxHeap(HeapCap);
                threadParameter->heap = heap;

                // delete poisonJob
                delete poisonJob;
                free(poison[0]);
                free(poison);

                // delete jobs array
                for (int i = 0; i < counter; i++) {
                    delete jobsArray[i];
                }
                free(jobsArray);
                // create new array
                jobsArray = (Job **) malloc(sizeof(Job *)*STARTSIZE);
                for (int i = 0; i < STARTSIZE; i++) {
                    jobsArray[i] = NULL;
                }
                currentSize = STARTSIZE;
                counter = 0;
            }

            for(int i = 0; i <= whitespace; i++){
                free(query[i]);
            }
            free(query);
            free(cline);
        }
        queryFile.close();

        free(jobsArray);
    }
    pthread_cond_destroy(&cond_nonempty);
    pthread_cond_destroy(&cond_nonfull);
    pthread_mutex_destroy(&printer_mtx);
    pthread_mutex_destroy(&heap_mtx);
    pthread_mutex_destroy(&queue_mtx);

    delete threadParameter;
}

int searchNgramStatic(char **query,int queryNum,Head *head){

    TrieNode *currentNode = NULL;
    int found = 0;
    int hash = hashcii(query[0]) % head->root->size;
    if(hash < head->root->split){
        hash = hashcii(query[0]) % head->root->newsize;
    }
    int wordSoFar = 0;
    int left = 0;
    int right = head->root->items[hash] - 1;
    while(left <= right){                       // checking if 1st word of Ngram exists and where
        int mid = left + ((right - left)/2);
        if(head->root->hashtable[hash] != NULL){
            if((*head->root->hashtable[hash])[mid].word != NULL){
                char *position = (*head->root->hashtable[hash])[mid].word;
                char *strings[(*head->root->hashtable[hash])[mid].compressedNum];
                for (int i = 0; i < (*head->root->hashtable[hash])[mid].compressedNum; i++) {
                    int templen = (*head->root->hashtable[hash])[mid].compressedLengths[i];
                    if(templen < 0){
                        templen = templen*(-1);
                    }
                    strings[i] = (char *) malloc(sizeof(char)*(templen+1));
                    strncpy(strings[i],position,templen);
                    strings[i][templen] = '\0';
                    if(i != (*head->root->hashtable[hash])[mid].compressedNum-1){
                        position = position + templen;
                    }
                }
                int counter = 0;
                for (int i = 0; i < (*head->root->hashtable[hash])[mid].compressedNum; i++) {
                    if(counter == queryNum){
                        wordSoFar = counter;
                        found = 1;
                        break;
                    }
                    if(strcmp(query[i],strings[i]) == 0){
                        counter++;
                        if(counter == (*head->root->hashtable[hash])[mid].compressedNum){
                            wordSoFar = (*head->root->hashtable[hash])[mid].compressedNum;
                            found = 1;
                        }
                    } else break;
                }
                if(found == 1){
                    for (int i = 0; i < (*head->root->hashtable[hash])[mid].compressedNum; i++) {
                        free(strings[i]);
                    }
                    currentNode = &(*head->root->hashtable[hash])[mid];
                    if(currentNode->compressedLengths[counter-1] < 0){
                        found = 2;
                    }
                    break;
                } else {
                    if((*head->root->hashtable[hash])[mid].compressedNum > 1 ){
                        int checkSize = (*head->root->hashtable[hash])[mid].compressedLengths[0];
                        if(checkSize < 0){
                            checkSize = checkSize*(-1);
                        }
                        char *word = (char *) malloc(sizeof(char)*(checkSize+1));
                        strncpy(word,(*head->root->hashtable[hash])[mid].word,checkSize);
                        word[checkSize] = '\0';
                        if(strcmp(word,query[0]) > 0){
                            right = mid - 1;
                            for (int i = 0; i < (*head->root->hashtable[hash])[mid].compressedNum; i++) {
                                free(strings[i]);
                            }
                            free(word);
                        } else if(strcmp(word,query[0]) < 0){
                            left = mid + 1;
                            for (int i = 0; i < (*head->root->hashtable[hash])[mid].compressedNum; i++) {
                                free(strings[i]);
                            }
                            free(word);
                        } else {
                            for (int i = 0; i < (*head->root->hashtable[hash])[mid].compressedNum; i++) {
                                free(strings[i]);
                            }
                            free(word);
                            break;
                        }
                    } else{
                        if(strcmp(query[0],(*head->root->hashtable[hash])[mid].word) < 0){
                            right = mid - 1;
                            for (int i = 0; i < (*head->root->hashtable[hash])[mid].compressedNum; i++) {
                                free(strings[i]);
                            }
                        } else if(strcmp(query[0],(*head->root->hashtable[hash])[mid].word) > 0){
                            left = mid + 1;
                            for (int i = 0; i < (*head->root->hashtable[hash])[mid].compressedNum; i++) {
                                free(strings[i]);
                            }
                        }
                    }
                }
            }
        }
    }
    if((found > 0) && (queryNum != wordSoFar)){
        for (int j = wordSoFar; j < queryNum; j++) {                // for each word of the Ngram
            if(wordSoFar == queryNum){
                break;
            }
            found = 0;
            int left = 0;
            int right = currentNode->childNum - 1;
            while(left <= right){                       // checking if i-th word of Ngram exists and where
                int mid = left + ((right - left)/2);
                if(currentNode->childs != NULL){
                    if((*currentNode->childs)[mid].word != NULL){
                        char *position = (*currentNode->childs)[mid].word;
                        char *strings[(*currentNode->childs)[mid].compressedNum];
                        for (int i = 0; i < (*currentNode->childs)[mid].compressedNum; i++) {
                            int templen = (*currentNode->childs)[mid].compressedLengths[i];
                            if(templen < 0){
                                templen = templen*(-1);
                            }
                            strings[i] = (char *) malloc(sizeof(char)*(templen+1));
                            strncpy(strings[i],position,templen);
                            strings[i][templen] = '\0';
                            if(i != (*currentNode->childs)[mid].compressedNum-1){
                                position = position + templen;
                            }
                        }
                        int counter = 0;
                        for (int i = 0; i < (*currentNode->childs)[mid].compressedNum; i++) {
                            if(wordSoFar + counter == queryNum){
                                wordSoFar = wordSoFar + counter;
                                found = 1;
                                break;
                            }
                            if(strcmp(query[i+wordSoFar],strings[i]) == 0){
                                counter++;
                                if(counter == (*currentNode->childs)[mid].compressedNum){
                                    wordSoFar = wordSoFar + (*currentNode->childs)[mid].compressedNum;
                                    j = j + (*currentNode->childs)[mid].compressedNum-1;
                                    found = 1;
                                }
                            } else break;
                        }
                        if(found == 1){
                            for (int i = 0; i < (*currentNode->childs)[mid].compressedNum; i++) {
                                free(strings[i]);
                            }
                            currentNode = &(*currentNode->childs)[mid];
                            if(currentNode->compressedLengths[counter-1] < 0){
                                found = 2;
                            }
                            break;
                        } else {
                            if((*currentNode->childs)[mid].compressedNum > 1 ){
                                int checkSize = (*currentNode->childs)[mid].compressedLengths[0];
                                if(checkSize < 0){
                                    checkSize = checkSize*(-1);
                                }
                                char *word = (char *) malloc(sizeof(char)*(checkSize+1));
                                strncpy(word,(*currentNode->childs)[mid].word,checkSize);
                                word[checkSize] = '\0';
                                if(strcmp(word,query[j]) > 0){
                                    right = mid - 1;
                                    for (int i = 0; i < (*currentNode->childs)[mid].compressedNum; i++) {
                                        free(strings[i]);
                                    }
                                    free(word);
                                } else if(strcmp(word,query[j]) < 0){
                                    left = mid + 1;
                                    for (int i = 0; i < (*currentNode->childs)[mid].compressedNum; i++) {
                                        free(strings[i]);
                                    }
                                    free(word);
                                } else {
                                    for (int i = 0; i < (*currentNode->childs)[mid].compressedNum; i++) {
                                        free(strings[i]);
                                    }
                                    free(word);
                                    break;
                                }
                            } else{
                                if(strcmp(query[j],(*currentNode->childs)[mid].word) < 0){
                                    right = mid - 1;
                                    for (int i = 0; i < (*currentNode->childs)[mid].compressedNum; i++) {
                                        free(strings[i]);
                                    }
                                } else if(strcmp(query[j],(*currentNode->childs)[mid].word) > 0){
                                    left = mid + 1;
                                    for (int i = 0; i < (*currentNode->childs)[mid].compressedNum; i++) {
                                        free(strings[i]);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if(found == 0){                     // if i-th word of Ngram does not exist
                return 0;
            }
        }
    }
    if(found == 1){
        return 1;
    } else if(found == 2){
        return 2;
    }

    return 0;
}

int sortCompressedChilds(TrieNode *currentNode,int x){

    int left = x+1;
    int right = currentNode->childNum-1;
    int mid = 0;
    while(left <= right){
        mid = left + ((right - left)/2);
        if(strcmp((*currentNode->childs)[x].word,(*currentNode->childs)[mid].word) < 0){
            right = mid - 1;
        } else if(strcmp((*currentNode->childs)[x].word,(*currentNode->childs)[mid].word) > 0){
            left = mid + 1;
        }
    }
    // moving inserted node to the right position in the array
    TrieNode *tempNode = (TrieNode *) malloc(sizeof(TrieNode)*(left-x-1));
    memcpy(tempNode,&(*currentNode->childs)[x+1],sizeof(TrieNode)*(left-x-1));
    memmove(&(*currentNode->childs)[left-1],&(*currentNode->childs)[x],sizeof(TrieNode));
    memcpy(&(*currentNode->childs)[x],tempNode,sizeof(TrieNode)*(left-x-1));
    free(tempNode);

    if(left-x-1 == 0){
        return -1;
    }
    return left-1;
}

int sortCompressedCells(RootNode *root,int hash,int x){

    int left = x+1;
    int right = root->items[hash]-1;
    int mid = 0;
    while(left <= right){
        mid = left + ((right - left)/2);
        if(strcmp((*root->hashtable[hash])[x].word,(*root->hashtable[hash])[mid].word) < 0){
            right = mid - 1;
        } else if(strcmp((*root->hashtable[hash])[x].word,(*root->hashtable[hash])[mid].word) > 0){
            left = mid + 1;
        }
    }
    // moving inserted node to the right position in the array
    TrieNode *tempNode = (TrieNode *) malloc(sizeof(TrieNode)*(left-x-1));
    memcpy(tempNode,&(*root->hashtable[hash])[x+1],sizeof(TrieNode)*(left-x-1));
    memmove(&(*root->hashtable[hash])[left-1],&(*root->hashtable[hash])[x],sizeof(TrieNode));
    memcpy(&(*root->hashtable[hash])[x],tempNode,sizeof(TrieNode)*(left-x-1));
    free(tempNode);

    if(left-x-1 == 0){
        return -1;
    }
    return left-1;
}
