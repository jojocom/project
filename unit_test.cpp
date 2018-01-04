#include <iostream>
#include <assert.h>
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
#include "heap.h"

using namespace std;

int main(int argc, char *argv[]) {

// check constructors of Head and TrieNode
    Head *head = new Head();
    assert(head != NULL);
    assert(head->root != NULL);
    assert(head->root->split == 0);
    assert(head->root->size == HashSize);
    assert(head->root->newsize == 2*HashSize);
    assert(head->root->currentSize == HashSize);
    assert(head->root->bucketSizes != NULL);
    assert(head->root->items != NULL);
    assert(head->root->hashtable != NULL);
    for (int i = 0; i < HashSize; i++) {
        assert(head->root->bucketSizes[i] == BucketLen);
    }
    for (int i = 0; i < HashSize; i++) {
        assert(head->root->items[i] == 0);
    }
    for (int j = 0; j < head->root->currentSize; j++) {
        for (int i = 0; i < head->root->bucketSizes[j]; i++) {
            assert((*head->root->hashtable[j])[i].capacity == N);
            assert((*head->root->hashtable[j])[i].childNum == 0);
            assert((*head->root->hashtable[j])[i].final == false);
            assert((*head->root->hashtable[j])[i].word == NULL);
            assert((*head->root->hashtable[j])[i].childs == NULL);
            assert((*head->root->hashtable[j])[i].compressedNum == 0);
            assert((*head->root->hashtable[j])[i].compressedLengths == NULL);
        }
    }
// check insertNgram
// Ngram of one word
    char word1[] = "this";
    char **query1 = new char*[1];
    query1[0] = word1;
    insertNgram(query1,1,head,0);
    int hash = hashcii(word1) % head->root->size;
    assert(head->root->items[hash] == 1);
    assert((*head->root->hashtable[hash])[head->root->items[hash]-1].capacity == N);
    assert((*head->root->hashtable[hash])[head->root->items[hash]-1].childNum == 0);
    assert((*head->root->hashtable[hash])[head->root->items[hash]-1].final == true);
    assert(strcmp((*head->root->hashtable[hash])[head->root->items[hash]-1].word,word1) == 0);
// Ngram of one word in the same bucket
    char word1a[] = "siht";
    char **query1a = new char*[1];
    query1a[0] = word1a;
    insertNgram(query1a,1,head,0);
    hash = hashcii(word1a) % head->root->size;
    assert(head->root->items[hash] == 2);
    assert((*head->root->hashtable[hash])[head->root->items[hash]-2].capacity == N);
    assert((*head->root->hashtable[hash])[head->root->items[hash]-2].childNum == 0);
    assert((*head->root->hashtable[hash])[head->root->items[hash]-2].final == true);
    assert(strcmp((*head->root->hashtable[hash])[head->root->items[hash]-2].word,word1a) == 0);
    assert((*head->root->hashtable[hash])[head->root->items[hash]-1].capacity == N);
    assert((*head->root->hashtable[hash])[head->root->items[hash]-1].childNum == 0);
    assert((*head->root->hashtable[hash])[head->root->items[hash]-1].final == true);
    assert(strcmp((*head->root->hashtable[hash])[head->root->items[hash]-1].word,word1) == 0);
// two Ngram of one word, double childs
    char word2[] = "the";
    char **query2 = new char*[1];
    query2[0] = word2;
    insertNgram(query2,1,head,0);
    hash = hashcii(word2) % head->root->size;
    assert(head->root->items[hash] == 1);
    assert(strcmp((*head->root->hashtable[hash])[head->root->items[hash]-1].word,word2) == 0);
    assert((*head->root->hashtable[hash])[head->root->items[hash]-1].final == true);
// Ngram of many words
    char word3[] = "is";
    char **query3 = new char*[2];
    query3[0] = word1;
    query3[1] = word3;
    insertNgram(query3,2,head,0);
    hash = hashcii(word1) % head->root->size;
    assert(head->root->items[hash] == 2);
    assert((*head->root->hashtable[hash])[head->root->items[hash]-1].capacity == N);
    assert((*head->root->hashtable[hash])[head->root->items[hash]-1].childNum == 1);
    assert((*head->root->hashtable[hash])[head->root->items[hash]-1].final == true);
    assert(strcmp((*head->root->hashtable[hash])[head->root->items[hash]-1].word,word1) == 0);
    assert(strcmp((*(*head->root->hashtable[hash])[head->root->items[hash]-1].childs)[0].word,word3) == 0);
    assert((*(*head->root->hashtable[hash])[head->root->items[hash]-1].childs)[0].final == true);
// check if that is final
    char word4[] = "that";
    char **query4 = new char*[2];
    query4[0] = word4;
    query4[1] = word3;
    insertNgram(query4,2,head,0);
    hash = hashcii(word4) % head->root->size;
    assert(head->root->items[hash] == 1);
    assert((*head->root->hashtable[hash])[head->root->items[hash]-1].capacity == N);
    assert((*head->root->hashtable[hash])[head->root->items[hash]-1].childNum == 1);
    assert((*head->root->hashtable[hash])[head->root->items[hash]-1].final == false);
    assert(strcmp((*head->root->hashtable[hash])[head->root->items[hash]-1].word,word4) == 0);
    assert(strcmp((*(*head->root->hashtable[hash])[head->root->items[hash]-1].childs)[0].word,word3) == 0);
    assert((*(*head->root->hashtable[hash])[head->root->items[hash]-1].childs)[0].final == true);
// check if that is final
    insertNgram(query4,1,head,0);
    assert((*head->root->hashtable[hash])[head->root->items[hash]-1].final == true);
// check deleteNgram
// delete Ngram that has child
    deleteNgram(query4,1,head,0);
    assert((*head->root->hashtable[hash])[head->root->items[hash]-1].final == false);
// delete Ngram with no childs
    deleteNgram(query4,2,head,0);
    assert(head->root->items[hash] == 0);
// delete Ngram that contains a node with childs
    insertNgram(query4,2,head,0);
    char word5[] = "was";
    char **query5 = new char*[2];
    query5[0] = word4;
    query5[1] = word5;
    insertNgram(query5,2,head,0);
    assert((*head->root->hashtable[hash])[head->root->items[hash]-1].childNum == 2);
    deleteNgram(query4,2,head,0);
    assert((*head->root->hashtable[hash])[head->root->items[hash]-1].childNum == 1);
    assert(strcmp((*head->root->hashtable[hash])[head->root->items[hash]-1].word,word4) == 0);
    assert(strcmp((*(*head->root->hashtable[hash])[head->root->items[hash]-1].childs)[0].word,word5) == 0);
// check searchNgram
    char **query6 = new char*[2];
    query6[0] = word1;
    query6[1] = word5;
    assert(searchNgram(query6,2,head,0) == 0);
    assert(searchNgram(query5,2,head,0) == 1);
// check sortChilds
    TrieNode *node = new TrieNode(6);
    node->childNum = 6;
    char worda[] = "a";
    char wordb[] = "b";
    char wordc[] = "c";
    char wordd[] = "d";
    char worde[] = "e";
    char wordf[] = "f";
    setNode(&(*node->childs)[0],N,false,worda,0,-1);
    setNode(&(*node->childs)[1],N,false,wordc,0,-1);
    setNode(&(*node->childs)[2],N,false,wordd,0,-1);
    setNode(&(*node->childs)[3],N,false,worde,0,-1);
    setNode(&(*node->childs)[4],N,false,wordf,0,-1);
    setNode(&(*node->childs)[5],N,false,wordb,0,-1);
    assert(sortChilds(node) == 1);
    assert(strcmp((*node->childs)[0].word,(char *)"a") == 0);
    assert(strcmp((*node->childs)[1].word,(char *)"b") == 0);
    assert(strcmp((*node->childs)[2].word,(char *)"c") == 0);
    assert(strcmp((*node->childs)[3].word,(char *)"d") == 0);
    assert(strcmp((*node->childs)[4].word,(char *)"e") == 0);
    assert(strcmp((*node->childs)[5].word,(char *)"f") == 0);
// check constructor of ListHead
    ListHead *listHead = new ListHead();
    assert(listHead->nodesNum == 0);
    assert(listHead->start == NULL);
// check insertListNode
// insert first Ngram
    listHead->insertListNode(query1,1);
    assert(listHead->nodesNum == 1);
    assert(listHead->start != NULL);
    assert(strcmp(listHead->start->query[0],word1) == 0);
    assert(listHead->start->next == NULL);
// insert same Ngram
    listHead->insertListNode(query1,1);
    assert(listHead->nodesNum == 1);
    assert(listHead->start != NULL);
    assert(strcmp(listHead->start->query[0],word1) == 0);
    assert(listHead->start->next == NULL);
// insert different Ngram
    listHead->insertListNode(query2,1);
    assert(listHead->nodesNum == 2);
    assert(listHead->start != NULL);
    assert(strcmp(listHead->start->query[0],word1) == 0);
    assert(listHead->start->next != NULL);
    assert(strcmp(listHead->start->next->query[0],word2) == 0);
    assert(listHead->start->next->next == NULL);
// check trie compress
    insertNgram(query4,2,head,0);
    trieCompress(head);
// node with one child
    hash = hashcii(word1) % head->root->size;
    assert(head->root->items[hash] == 2);
    assert(strcmp((*head->root->hashtable[hash])[head->root->items[hash]-2].word,word1a) == 0);
    assert((*head->root->hashtable[hash])[head->root->items[hash]-2].compressedNum == 1);
    assert((*head->root->hashtable[hash])[head->root->items[hash]-2].compressedLengths[0] == 4);
    assert(strcmp((*head->root->hashtable[hash])[head->root->items[hash]-1].word,"thisis") == 0);
    assert((*head->root->hashtable[hash])[head->root->items[hash]-1].compressedNum == 2);
    assert((*head->root->hashtable[hash])[head->root->items[hash]-1].compressedLengths[0] == 4);
    assert((*head->root->hashtable[hash])[head->root->items[hash]-1].compressedLengths[1] == 2);
// node with two childs
    hash = hashcii(word4) % head->root->size;
    assert(head->root->items[hash] == 1);
    assert(strcmp((*head->root->hashtable[hash])[head->root->items[hash]-1].word,word4) == 0);
    assert((*head->root->hashtable[hash])[head->root->items[hash]-1].compressedNum == 1);
    assert((*head->root->hashtable[hash])[head->root->items[hash]-1].compressedLengths[0] == -4);
    assert(strcmp((*(*head->root->hashtable[hash])[head->root->items[hash]-1].childs)[0].word,word3) == 0);
    assert((*(*head->root->hashtable[hash])[head->root->items[hash]-1].childs)[0].compressedNum == 1);
    assert((*(*head->root->hashtable[hash])[head->root->items[hash]-1].childs)[0].compressedLengths[0] == 2);
    assert(strcmp((*(*head->root->hashtable[hash])[head->root->items[hash]-1].childs)[1].word,word5) == 0);
    assert((*(*head->root->hashtable[hash])[head->root->items[hash]-1].childs)[1].compressedNum == 1);
    assert((*(*head->root->hashtable[hash])[head->root->items[hash]-1].childs)[1].compressedLengths[0] == 3);

    delete [] query1;
    delete [] query1a;
    delete [] query2;
    delete [] query3;
    delete [] query4;
    delete [] query5;
    delete [] query6;

    delete listHead;
    delete node;
    delete head;

// check heap
    MaxHeap *heap = new MaxHeap(2);

    assert(heap->elements == 0);

    char *heapword1 = (char *) malloc(sizeof(char)*(strlen("garcia")+1));
    strcpy(heapword1,"garcia");
    char *heapword2 = (char *) malloc(sizeof(char)*(strlen("above")+1));
    strcpy(heapword2,"above");
// check heap insert
    heap->insertKey(heapword1);
    assert(heap->elements == 1);
    heap->insertKey(heapword2);
    assert(heap->elements == 2);
// check heap extract
    Element *el1 = heap->extractMax();
    assert(el1 != NULL);
    assert(heap->elements == 1);
    assert(strcmp(el1->word,heapword2) == 0);
    assert(el1->occurrences == 1);
    delete el1;

    heap->insertKey(heapword1);
    assert(heap->elements == 1);
    heap->insertKey(heapword2);
    assert(heap->elements == 2);

    el1 = heap->extractMax();
    assert(el1 != NULL);
    assert(heap->elements == 1);
    assert(strcmp(el1->word,heapword1) == 0);
    assert(el1->occurrences == 2);
    delete el1;

    free(heapword1);
    free(heapword2);
    delete heap;

    return 0;
}
