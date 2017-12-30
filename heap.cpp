#include <iostream>
#include <climits>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <stdint.h>
#include "heap.h"

using namespace std;

Element::Element() : occurrences(0),word(NULL){}

Element::~Element(){
    if(word != NULL){
        free(word);
    }
}

void MaxHeap::setNode(Element *element,int occurrences,char *word){
    if(word != NULL){
        element->word = (char *) malloc(sizeof(char)*(strlen(word)+1));
        strcpy(element->word,word);
    } else{
        element->word = NULL;
    }
    element->occurrences = occurrences;
}

// Constructor: Builds a heap from a given array a[] of given size
MaxHeap::MaxHeap(int cap) : elements(0),size(cap){
    array = (Element **) malloc(sizeof(Element*));
    *array = (Element *) malloc(sizeof(Element)*cap);
    for(int i = 0; i < cap; i++){
        setNode(&(*array)[i],0,NULL);
    }
}

MaxHeap::~MaxHeap(){
    for (int i = 0; i < size; i++) {
        if((*array)[i].word != NULL){
            free((*array)[i].word);
        }
    }
    free(*array);
    free(array);
}

// A recursive method to heapify a subtree with root at given index
// This method assumes that the subtrees are already heapified
void MaxHeap::MaxHeapify(int i){
    int l = left(i);
    int r = right(i);
    int largest = i;

    if (l < elements && (*array)[l].occurrences >= (*array)[i].occurrences){
        if((*array)[l].occurrences == (*array)[i].occurrences){
            if(strcmp((*array)[l].word,(*array)[i].word) > 0){
                largest = i;
            } else{
                largest = l;
            }
        } else{
            largest = l;
        }
    }
    if (r < elements && (*array)[r].occurrences >= (*array)[largest].occurrences){
        if((*array)[r].occurrences == (*array)[largest].occurrences){
            if(strcmp((*array)[r].word,(*array)[largest].word) < 0){
                largest = r;
            }
        } else{
            largest = r;
        }
    }
    if (largest != i){
        swap(&(*array)[i], &(*array)[largest]);
        MaxHeapify(largest);
    }
}

// Inserts a new key 'k'
void MaxHeap::insertKey(char *k){
    int i = elements - 1;
    if((i = searchHeap(k,0)) == -1){
        if (elements == size){
            *array = (Element *) realloc(*array,sizeof(Element)*2*size);
            for (int i = size; i < 2*size; i++) {
                setNode(&(*array)[i],0,NULL);
            }
            size = 2*size;
        }
        // First insert the new key at the end
        elements++;
        i = elements - 1;
        (*array)[i].word = (char *) malloc(sizeof(char)*(strlen(k)+1));
        strcpy((*array)[i].word,k);
        (*array)[i].occurrences = 1;

    }
    // Fix the max heap property if it is violated
    while (i != 0 && ((*array)[parent(i)].occurrences <= (*array)[i].occurrences)){
        if((*array)[parent(i)].occurrences == (*array)[i].occurrences){
            if(strcmp((*array)[parent(i)].word,(*array)[i].word) > 0){
                swap(&(*array)[i], &(*array)[parent(i)]);
                i = parent(i);
            } else break;
        }else{
            swap(&(*array)[i], &(*array)[parent(i)]);
            i = parent(i);
        }
    }
}

int MaxHeap::searchHeap(char *neWord,int i) {
    int x = -1;
    if (i < elements) {
        if(strcmp(neWord,(*array)[i].word) == 0) {
            (*array)[i].occurrences++;
            return i;
        }
        if ((left(i) < elements) && x == -1) {
            x = searchHeap(neWord,left(i));
        }
        if ((right(i) < elements) && x == -1){
            x = searchHeap(neWord,right(i));
        }
    }
    return x;
}

// Method to remove maximum element (or root) from min heap
Element *MaxHeap::extractMax(){
    if (elements <= 0)
        return NULL;
    if (elements == 1)
    {
        elements--;
        return &(*array[0]);
    }

    // Store the maximum value, and remove it from heap
    Element *root = new Element();
    setNode(root,(*array)[0].occurrences,(*array)[0].word);

    free((*array)[0].word);
    (*array)[0].word = NULL;
    setNode(&(*array)[0],(*array)[elements-1].occurrences,(*array)[elements-1].word);
    if((*array)[elements-1].word != NULL){
        free((*array)[elements-1].word);
        (*array)[elements-1].word = NULL;
    }

    elements--;
    MaxHeapify(0);

    return root;
}

void MaxHeap::printHeap(int i,int tabs){
    if (i < elements) {
        for (int j = 0; j < tabs; j++) {
            cout << "\t";
        }
        cout << (*array)[i].word << " " << (*array)[i].occurrences << endl;
        if (left(i) < elements) {
            printHeap(left(i),tabs+1);
        }
        if (right(i) < elements) {
            printHeap(right(i),tabs+1);
        }
    }
}

// A utility function to swap two elements
void swap(Element *x, Element *y){
	Element *temp = (Element *) malloc(sizeof(Element));
    memcpy(temp,x,sizeof(Element));
    memcpy(x,y,sizeof(Element));
    memcpy(y,temp,sizeof(Element));
    free(temp);
}
