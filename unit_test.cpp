#include <iostream>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include "trie.h"
#include "list.h"

using namespace std;

int main(int argc, char *argv[]) {

// check constructors of Head and TrieNode
    Head *head = new Head();
    assert(head != NULL);
    assert(head->root != NULL);
    assert(head->root->word == NULL);
    assert(head->root->capacity == N);
    assert(head->root->childNum == 0);
    assert(head->root->childs != NULL);
    assert((*head->root->childs) != NULL);
    for (int i = 0; i < head->root->capacity; i++) {
        assert((*head->root->childs)[i].capacity == N);
        assert((*head->root->childs)[i].childNum == 0);
        assert((*head->root->childs)[i].final == false);
        assert((*head->root->childs)[i].word == NULL);
    }
// check insertNgram
// Ngram of one word
    char word1[] = "this";
    char **query1 = new char*[1];
    query1[0] = word1;
    insertNgram(query1,1,head);

    assert(head->root->childNum == 1);
    assert((*head->root->childs)[head->root->childNum-1].capacity == N);
    assert((*head->root->childs)[head->root->childNum-1].childNum == 0);
    assert((*head->root->childs)[head->root->childNum-1].final == true);
    assert(strcmp((*head->root->childs)[head->root->childNum-1].word,word1) == 0);
// two Ngram of one word, double childs
    char word2[] = "the";
    char **query2 = new char*[1];
    query2[0] = word2;
    insertNgram(query2,1,head);

    assert(head->root->capacity == 2);
    assert(head->root->childNum == 2);
    assert(strcmp((*head->root->childs)[head->root->childNum-1].word,word1) == 0);
    assert((*head->root->childs)[head->root->childNum-1].final == true);
    assert(strcmp((*head->root->childs)[head->root->childNum-2].word,word2) == 0);
    assert((*head->root->childs)[head->root->childNum-2].final == true);
// Ngram of many words
    char word3[] = "that";
    char word4[] = "is";
    char **query3 = new char*[2];
    query3[0] = word3;
    query3[1] = word4;
    insertNgram(query3,2,head);

    assert(head->root->capacity == 4);
    assert(head->root->childNum == 3);
    assert(strcmp((*head->root->childs)[head->root->childNum-3].word,word3) == 0);
    assert((*head->root->childs)[head->root->childNum-3].final == false);
    assert(strcmp((*head->root->childs)[head->root->childNum-1].word,word1) == 0);
    assert((*head->root->childs)[head->root->childNum-1].final == true);
    assert(strcmp((*head->root->childs)[head->root->childNum-2].word,word2) == 0);
    assert((*head->root->childs)[head->root->childNum-2].final == true);
    assert(strcmp((*head->root->childs[head->root->childNum-3]->childs)[0].word,word4) == 0);
    assert((*head->root->childs[head->root->childNum-3]->childs)[0].final == true);
// check if that is final
    insertNgram(query3,1,head);
    assert((*head->root->childs)[head->root->childNum-3].final == true);
// check deleteNgram
// delete Ngram that has child
    deleteNgram(query3,1,head);
    assert((*head->root->childs)[head->root->childNum-3].final == false);
// delete Ngram with no childs
    deleteNgram(query3,2,head);
    assert(head->root->childNum == 2);
    assert(strcmp((*head->root->childs)[head->root->childNum-1].word,word1) == 0);
    assert(strcmp((*head->root->childs)[head->root->childNum-2].word,word2) == 0);
// delete Ngram that contains a node with childs
    insertNgram(query3,2,head);
    char word5[] = "was";
    char **query4 = new char*[2];
    query4[0] = word3;
    query4[1] = word5;
    insertNgram(query4,2,head);
    assert((*head->root->childs)[head->root->childNum-3].childNum == 2);
    deleteNgram(query3,2,head);
    assert(strcmp((*head->root->childs)[head->root->childNum-3].word,word3) == 0);
    assert(head->root->childs[head->root->childNum-3]->childNum == 1);
    assert(strcmp((*head->root->childs[head->root->childNum-3]->childs)[0].word,word5) == 0);
// check searchNgram
    char **query5 = new char*[2];
    query5[0] = word1;
    query5[1] = word5;
    assert(searchNgram(query5,2,head) == 0);
    assert(searchNgram(query4,2,head) == 1);
// check sortChilds
    TrieNode *node = new TrieNode(6);
    node->childNum = 6;
    char worda[] = "a";
    char wordb[] = "b";
    char wordc[] = "c";
    char wordd[] = "d";
    char worde[] = "e";
    char wordf[] = "f";
    setNode(&(*node->childs)[0],N,false,worda);
    setNode(&(*node->childs)[1],N,false,wordc);
    setNode(&(*node->childs)[2],N,false,wordd);
    setNode(&(*node->childs)[3],N,false,worde);
    setNode(&(*node->childs)[4],N,false,wordf);
    setNode(&(*node->childs)[5],N,false,wordb);
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

    delete [] query1;
    delete [] query2;
    delete [] query3;
    delete [] query4;
    delete [] query5;

    delete listHead;
    delete node;
    delete head;

    return 0;
}
