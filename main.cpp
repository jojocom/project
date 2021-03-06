#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include "trie.h"
// #include "jobScheduler.h"

using namespace std;

int main(int argc, char *argv[]) {                                              // ./ngrams -i <init_file> -q <query_file>
    char * initFileName = NULL;
    char * queryFileName = NULL;
    int check1 = 0, check2 = 0;

// reading parameters
    for(int looper=1 ; looper<argc ; looper = looper+2) {
        if(strcmp("-i",argv[looper]) == 0) {
            initFileName = new char[(strlen(argv[looper+1]) +1)];
            strcpy(initFileName,argv[looper+1]);
            check1 = 1;
        }
        else if(strcmp("-q",argv[looper]) == 0) {
            queryFileName = new char[(strlen(argv[looper+1]) +1)];
            strcpy(queryFileName,argv[looper+1]);
            check2 = 1;
        }
    }
// check parameters
    if( argc != 5 || check1 == 0 || check2 == 0) {
        cout << "No correct arguments provided!" << endl;
        cout << "Right usage: ./ngrams -i <init_file> -q <query_file>" << endl;
        return 0;
    }
    // create trie head
    int compress = 0;
    Head *head;
    head = trieCreate(initFileName,&compress);

    if(compress == 0){
        // reading queries
        // std::cout << "main" << '\n';
        queryRead(queryFileName,head);
        // print(head->root);
    } else{
        trieCompress(head);
        queryStaticRead(queryFileName,head);
    }

    delete [] initFileName;
    delete [] queryFileName;
    delete head;

    return 0;
}
