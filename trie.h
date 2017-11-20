#define N 10

class TrieNode {
public:
    int capacity;
    int childNum;
    bool final;
    char * word;
    TrieNode **childs;

    TrieNode();
    TrieNode(int capacity);
    ~TrieNode();
};

class Head {
public:
    TrieNode * root;
    Head();
    ~Head();
};

void setNode(TrieNode *current,int capacity,bool final,char *word);
void free_trie(TrieNode *current);
Head * trieCreate(char *fileName);
void insertNgram(char **query,int queryNum,Head *head);
int searchNgram(char **query,int queryNum,Head *head);
void deleteNgram(char **query,int queryNum,Head *head);
void queryRead(char *queryFileName,Head *head);
void printAll(TrieNode *root,int tabs);
int sortChilds(TrieNode *currentNode);
