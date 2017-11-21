#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <stdint.h>
#include "trie.h"
#include "list.h"
#include "hash_functions.h"

#define M 25000

using namespace std;
// constructor of trie nodes
TrieNode::TrieNode(int capacity) : capacity(capacity), childNum(0), final(false){
    word = NULL;
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
                    // delete [] (*childs)[i].word;
                    free((*childs)[i].word);
                }
            }
            free(*childs);
            free(childs);
        }
    }
}

// set values of trie node members
void setNode(TrieNode *current,int capacity,bool final,char *word){
    current->capacity = capacity;
    current->childNum = 0;
    current->final = final;
    if(word != NULL){
        // current->word = new char[strlen(word)+1];
        current->word = (char *) malloc(sizeof(char)*(strlen(word) + 1));
        strcpy(current->word,word);
    } else {
        current->word = NULL;
    }
    current->childs = NULL;
}

// constructor of head
Head::Head(){
    root = new TrieNode(N);
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
    }
    free(*current->childs);             // delete array of childs
    free(current->childs);              // delete pointer to array of childs
}

// initializing of trie with Ngrams
Head * trieCreate(char *fileName){

    Head *head = new Head();
    string line;
    ifstream queryFile( fileName );

    if (queryFile) {
        while (getline( queryFile, line )) {        // read Ngram
            char *cline = (char *) malloc(sizeof(char)*(line.length() + 1));
            strcpy(cline, line.c_str());
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
    currentNode = head->root;
    for (int i = 0; i < queryNum; i++) {                // for each word of the Ngram
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

    // cout << "searchNgram Start" << endl;

    TrieNode *currentNode = NULL;
    int found = 0;

    currentNode = head->root;

    for (int i = 0; i < queryNum; i++) {                // for each word of the Ngram
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
    if(found == 1){
        return 1;
    } else if(found == 2){
        return 2;
    }

    return 0;
}

// deleting Ngram from the trie
void deleteNgram(char **query,int queryNum,Head *head){

    // cout << "deleteNgram Start" << endl;

    TrieNode **path = new TrieNode*[queryNum+1];
    TrieNode *currentNode = NULL;
    int found = 0;

    currentNode = head->root;
    path[0] = currentNode;
    for (int i = 0; i < queryNum; i++) {                // for each word of the Ngram
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
                        path[i+1] = currentNode;
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
    for (int i = queryNum + 1; i > 0; i--) {                // deleting node of i-th word of Ngram
        if(i == queryNum + 1){                      // if node of last word of Ngram
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
    for (int j = 0; j < queryNum; j++) {
        path[j] = NULL;
    }
    delete [] path;
    return ;
}

// reading queries and do Q,A or D
void queryRead(char *queryFileName,Head *head){

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
            bool bitArray[M];
            for (int i = 0; i < M; i++) {
                bitArray[i] = 0;
            }
            // ListHead *listHead = new ListHead();            // creating list for print
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
                                }
                            }
                            int hash1 = murmurhash(mykey,(uint32_t) strlen(mykey),0) % M;
                            int hash2 = hash_pearson(mykey) % M;
                            int hash3 = hash_jenkins(mykey) % M;

                            if((bitArray[hash1] == 0) || (bitArray[hash2] == 0) || (bitArray[hash3] == 0) || (bitArray[(hash3+hash1)%M] == 0) || (bitArray[(hash1+hash2)%M] == 0)){
                                bitArray[hash1] = 1;
                                bitArray[hash2] = 1;
                                bitArray[hash3] = 1;
                                bitArray[(hash3+hash1)%M] = 1;
                                bitArray[(hash1+hash2)%M] = 1;

                                if(found == 0){
                                    cout << mykey;
                                } else{
                                    cout << "|";
                                    cout << mykey;
                                }
                                // cout << "|";
                                // cout << mykey;
                            }
                            free(mykey);
                            // listHead->insertListNode(temp,j);   //insert Ngram to the list
                            found = 1;
                        } else if(x == 0){
                            break;
                        }
                    }
                }
                if(found == 0){
                    cout << "-1";               // if there are no Ngrams int the query
                }
                // listHead->printList();              // printing Ngrams int the query, one time each one
                cout << endl;
            }
            else if (strcmp(query[0],"A") == 0){              // for A query
                insertNgram(queryStart,whitespace,head);
            }
            else if (strcmp(query[0],"D") == 0){              // for D query
                deleteNgram(queryStart,whitespace,head);
            }
            else if (strcmp(query[0],"P") == 0){
                printAll(head->root,0);
            }

            // delete listHead;

            for(int i = 0; i <= whitespace; i++){
                free(query[i]);
            }
            free(query);
            free(cline);
        }
        queryFile.close();
    }
}

// printing of trie
void printAll(TrieNode *root,int tabs){
    if(root->word != NULL){
        for (int i = 0; i < tabs; i++) {
            cout << "\t";
        }
        cout << root->word << " " << root->final << " " << root->capacity << " " << root->childNum << endl;
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
