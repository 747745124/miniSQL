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

    //�x����ϵ�ӛ�
    vector<Tuple> selectRecord(const string &tablename, vector<Compare> &compare);

    //�h�����ϵ�ӛ�
    void deleteRecord(const string &tablename, vector<Compare> &compare);

    //����ӛ�,�������Ƿ�ɹ�
    bool insertRecord(const string &tablename, const Tuple &tuple);

    //�������
    bool createTable(const string &tablename, const vector<Attribute> &attribute, const string &primary = "");

    //�hȥ���
    bool dropTable(const string &tablename);

    //����index
    bool createIndex(const string &tablename, const string &indexname, const string &attrname);

    //�h��index
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