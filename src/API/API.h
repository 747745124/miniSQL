#ifndef _API_H_
#define _API_H_

#include "../BufferManager/bufferPoolManager.h"
#include "../CatalogManager/CatalogManager.h"
#include "../IndexManager/IndexManager.h"
#include "../RecordManager/RecordManager.h"

using namespace std;

class API
{
public:
    API();

    ~API();

    //選擇符合的記錄
    vector<Tuple> selectRecord(const string &tablename, vector<Compare> &compare);

    //刪除符合的記錄
    void deleteRecord(const string &tablename, vector<Compare> &compare);

    //插入記錄,會返回是否成功
    bool insertRecord(const string &tablename, const Tuple &tuple);

    //新增表格
    bool createTable(const string &tablename, const vector<Attribute> &attribute, const string &primary = "");

    //刪去表格
    bool dropTable(const string &tablename);

    //新增index
    bool createIndex(const string &tablename, const string &indexname, const string &attrname);

    //刪除index
    bool dropIndex(const string &indexname);

    BufferPoolManager *getBufferManager() { return bufferPoolManager; }

    IndexManager *getIndexManager() { return indexManager; }

    CatalogManager *getCatalogManager() { return catalogManager; }

private:
    CatalogManager *catalogManager;
    IndexManager *indexManager;
    RecordManager *recordManager;
    BufferPoolManager *bufferPoolManager;
};

#endif