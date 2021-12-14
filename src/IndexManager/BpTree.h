#ifndef MINISQL_BPTREE_H
#define MINISQL_BPTREE_H

#include <string>

#include "../basic.h"
#include "BpTreeNode.h"
#include "../BufferManager/bufferPoolManager.h"

using std::string;

struct BpTreeHeader {
    // 保存在文件
    int nodeCount, keySize; // 4+4 Bytes
    KeyType keyType; // 4 Bytes
    PageIndex rootPage; // 4 Bytes
};

class BpTree {
private:
    string tableName, indexName;
    string fileName;
    BpTreeHeader header{};


public:
    BufferPoolManager *buffer;

    bool insertKey(const BpKey &key);

    bool removeKey(const BpKey &key);

    BpTree(BufferPoolManager *bufferPoolManager, const string &tableName, const string &indexName, const int &indexType = -1);

    void search(const BpKey &left, const BpKey &right, vector<Position> &positions);

    void search(const BpKey &equal, vector<Position> &positions);

private:
    void convertHeaderFromData(Byte *data);

    void writeHeaderToData(Byte *data) const;

    BpTreeNode *createNode(bool isLeaf = false);

    void deleteNode(BpTreeNode *node);

    BpTreeNode *getNode(PageIndex pageIndex);

    BpTreeNode *getNode(unordered_map<PageIndex, BpTreeNode *> &nodeMap, PageIndex pageIndex);

    BpTreeNode *getLNode(unordered_map<PageIndex, BpTreeNode *> &nodeMap, BpTreeNode *node);

    BpTreeNode *getRNode(unordered_map<PageIndex, BpTreeNode *> &nodeMap, BpTreeNode *node);

    void splitNode(unordered_map<PageIndex, BpTreeNode *> &nodeMap,
                   BpTreeNode *x, BpTreeNode *y);

    void updateNode(unordered_map<PageIndex, BpTreeNode *> &nodeMap,
                    PageIndex pageIndex, const BpKey &key1, const BpKey &key2);

    void leafBorrowLeft(unordered_map<PageIndex, BpTreeNode *> &nodeMap,
                        BpTreeNode *node, BpTreeNode *lNode);

    void leafBorrowRight(unordered_map<PageIndex, BpTreeNode *> &nodeMap,
                         BpTreeNode *node, BpTreeNode *rNode);

    void nonleafBorrowLeft(unordered_map<PageIndex, BpTreeNode *> &nodeMap,
                           BpTreeNode *node, BpTreeNode *lNode);

    void nonleafBorrowRight(unordered_map<PageIndex, BpTreeNode *> &nodeMap,
                            BpTreeNode *node, BpTreeNode *rNode);

    void leafMerge(unordered_map<PageIndex, BpTreeNode *> &nodeMap,
                   BpTreeNode *lNode, BpTreeNode *rNode);

    void nonleafMerge(unordered_map<PageIndex, BpTreeNode *> &nodeMap,
                      BpTreeNode *lNode, BpTreeNode *rNode);


/* ---------- 以下为调试用代码，输出BpTree内容 ---------- */
/* ---------- 以下为调试用代码，输出BpTree内容 ---------- */
/* ---------- 以下为调试用代码，输出BpTree内容 ---------- */
public:
    void printAllTree();

    void printAllLeaf();

    bool check();
};


#endif //MINISQL_BPTREE_H
