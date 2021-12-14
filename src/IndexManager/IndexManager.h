#ifndef MINISQL_INDEXMANAGER_H
#define MINISQL_INDEXMANAGER_H

#include <string>
#include <vector>
#include <map>
using namespace std;

#include "../basic.h"
#include "BpTree.h"
#include "../RecordManager/RecordManager.h"
#include "../BufferManager/bufferPoolManager.h"
#include "../CatalogManager/CatalogManager.h"

class RecordManager;
class BufferPoolManager;
class CatalogManager;

class IndexManager {
private:
    map<pair<string, string>, BpTree *> treeMap;

    BufferPoolManager *buffer;
    RecordManager *record;
    CatalogManager *catalog;

    bool createIndex(const std::string &tableName, const std::string &indexName,
                     const std::vector<Data> &datas, const vector<Position> &positions);

public:

    void setPointer(BufferPoolManager *buffer = nullptr, RecordManager *record = nullptr,
                    CatalogManager *catalog = nullptr) {
        this->buffer = buffer;
        this->record = record;
        this->catalog = catalog;
    }

    IndexManager();

    ~IndexManager();

    /*
     * 为tableName表的属性attrName建立名为indexName的索引
     * 保存文件名为'{tableName}_{indexName}'
     * 获取包含所有条目的vector，然后通过这个vector建立B+树
     * 返回创建成功
     */
    bool createIndex(const std::string &tableName, const std::string &indexName,
                     const std::string &attrName);


    /*
     * 删除tableName表的indexName属性
     */
    bool dropIndex(const std::string &tableName, const std::string &indexName);

    /*
     * 在表table的索引index中插入一条，
     * 返回插入成功
     */
    bool insert(const std::string &tableName, const std::string &indexName,
                const Data &data, Position position);

    /*
     * 删除索引
     * record删除记录的时候顺带用这个把索引删了
     */
    bool remove(const std::string &tableName, const std::string &indexName,
                const Data &data);

    /*
     * 查找
     * left和right的type<-1或者>255就是没有限制
     * 俩都没有就不反回了
     * 左右都是闭区间！！！！！返回之后还要再处理
     * 非unique的时候<=可能出错！！！
     * 不过测试数据里面没有
     */
    std::vector<Position>
    search(const string &tableName, const string &indexName,
           const Data &left, const Data &right);

    std::vector<Position>
    search(const string &tableName, const string &indexName,
           const Data &equal);

    void printTree(const string &tableName, const string &indexName) {
        BpTree *tree = treeMap[make_pair(tableName, indexName)];
        if (tree == nullptr)
            tree = treeMap[make_pair(tableName, indexName)] = new BpTree(buffer, tableName, indexName);
        tree->printAllTree();
    }
};

#endif  // MINISQL_INDEXMANAGER_H
