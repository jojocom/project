#define N 10
#define M 3000
#define HashSize 50
#define BucketLen 4

class TrieNode {
public:
    int capacity;
    int childNum;
    bool final;
    char * word;
    TrieNode **childs;
    int compressedNum;
    short *compressedLengths;
    int add;
    int del;

    TrieNode();
    TrieNode(int capacity);
    ~TrieNode();
};

class RootNode {
public:
    int split;
    int size;
    int newsize;
    int currentSize;
    int *bucketSizes;
    int *items;
    TrieNode ***hashtable;

    RootNode();
    ~RootNode();
};

class Head {
public:
    RootNode * root;
    Head();
    ~Head();
};

void setNode(TrieNode *current,int capacity,bool final,char *word,int add,int del);
void free_trie(TrieNode *current);
Head * trieCreate(char *fileName,int *compress);
void insertNgram(char **query,int queryNum,Head *head,int counter);
void noInsertNgram(char **query,int queryNum,Head *head,int counter);
int searchNgram(char **query,int queryNum,Head *head);
void deleteNgram(char **query,int queryNum,Head *head,int counter);
void noDeleteNgram(char **query,int queryNum,Head *head,int counter);
void queryRead(char *queryFileName,Head *head);
void print(RootNode *root);
void printAll(TrieNode *root,int tabs);
int sortChilds(TrieNode *currentNode);
int sortCell(RootNode *root,int hash);
void trieCompress(Head *head);
void recursiveCompress(TrieNode *current);
void queryStaticRead(char *queryFileName,Head *head);
int searchNgramStatic(char **query,int queryNum,Head *head);
int sortCompressedChilds(TrieNode *currentNode,int x);
int sortCompressedCells(RootNode *root,int hash,int x);
