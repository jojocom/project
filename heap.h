#define HeapCap 20

class Element{
public:
    int occurrences;
    char *word;

    Element();
    ~Element();
};

// A class for Max Heap
class MaxHeap{
public:
	Element **array; // pointer to array of elements in heap
    int elements;
	int size; // Current number of elements in max heap

    void setNode(Element *element,int occurrences,char *word);
	MaxHeap(int cap); // Constructor
    ~MaxHeap(); // Destructor
    void MaxHeapify(int cap); // to heapify a subtree with root at given index
    void insertKey(char *k);   // Inserts a new key 'k'
    int searchHeap(char *neWord,int i);
    Element *extractMax();  // to extract the root which is the max element
    void printHeap(int i, int tabs);
    int parent(int i) { return (i-1)/2; }
    int left(int i) { return (2*i + 1); }  // to get index of left child of node at index i
    int right(int i) { return (2*i + 2); } // to get index of right child of node at index i
};

// Prototype of a utility function to swap two integers
void swap(Element *x, Element *y);
