#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include "trie.h"

using namespace std;

int main(int argc, char *argv[]) {                                              // ./ngrams -i <init_file> -q <query_file>
    char * initFileName;
    char * queryFileName;
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
    Head *head;
    head = trieCreate(initFileName);

    // reading queries
    queryRead(queryFileName,head);

    delete [] initFileName;
    delete [] queryFileName;
    delete head;

    return 0;
}
