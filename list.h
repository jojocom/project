using namespace std;

//----------------------------- ListNode ---------------------------------------

class ListNode {
public:
    char **query;
    int queryNum;
    ListNode *next;

    ListNode(char **newQuery,int newQueryNum);
    ~ListNode();
};

class ListHead {
public:

    int nodesNum;
    ListNode *start;

    ListHead();
    ~ListHead();

    void insertListNode(char **newQuery,int newQueryNum);
    void printList();
};
