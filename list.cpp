#include <iostream>
#include <string.h>
#include "list.h"

using namespace std;


ListNode::ListNode(char **newQuery,int newQueryNum) : queryNum(newQueryNum),next(NULL) {
    query = new char*[newQueryNum];
    for (int i = 0; i < newQueryNum; i++) {
        query[i] = new char[strlen(newQuery[i]) + 1];
        strcpy(query[i],newQuery[i]);
    }
}

ListNode::~ListNode() {
    if (query != NULL) {
        for (int i = 0; i < queryNum; i++) {
            delete [] query[i];
        }
        delete [] query;
    }
    if (next != NULL) {
        delete next;
    }
}

//------------------------------------------------------------------------------

ListHead::ListHead() : nodesNum(0), start(NULL) {
    //
}

ListHead::~ListHead() {
    if (start != NULL) {
        delete start;
    }
}

void ListHead::insertListNode(char **newQuery,int newQueryNum) {
    if (start == NULL) {
        start = new ListNode(newQuery,newQueryNum);
        nodesNum++;
    } else {
        ListNode *currentNode = start;
        int counter = 0;
        int found = 0;
        while (currentNode != NULL) {
            counter = 0;
            if( currentNode->queryNum == newQueryNum){
                for (int i = 0; i < newQueryNum; i++) {
                    if(strcmp(currentNode->query[i],newQuery[i]) == 0){
                        counter++;
                    } else{
                        break;
                    }
                }
            }
            if(counter == newQueryNum){
                found = 1;
                break;
            } else{
                if(currentNode->next != NULL){
                    currentNode = currentNode->next;
                } else {
                    break;
                }
            }
        }
        if(found == 0){
            currentNode->next = new ListNode(newQuery,newQueryNum);
            nodesNum++;
        }
    }
}

void ListHead::printList(){

    ListNode *currentNode = start;

    for (int j = 0; j < nodesNum; j++) {
        for (int i = 0; i < currentNode->queryNum; i++) {
            if(i == currentNode->queryNum - 1){
                cout << currentNode->query[i];
            } else{
                cout << currentNode->query[i] << " ";
            }
        }
        if((j != nodesNum - 1) && (nodesNum != 1)){
            cout << "|";
        }
        currentNode = currentNode->next;
    }
}
