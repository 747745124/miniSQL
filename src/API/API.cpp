#include "API.h"

API::API()
{
    bufferPoolManager = new BufferPoolManager(BUFFER_POOL_SIZE);
    recordManager = new RecordManager();
    indexManager = new IndexManager();
    catalogManager = new CatalogManager();

    recordManager->setPointer(indexManager, catalogManager, bufferPoolManager);
    indexManager->setPointer(bufferPoolManager, recordManager, catalogManager);
    catalogManager->setPointer(indexManager);
}

vector<Tuple> API::selectRecord(const string &tablename, vector<Compare> &compare)
{
    if (!catalogManager->isTableExist(tablename))
        throw runtime_error("Error: Table " + tablename + " doesn't exist.");
    vector<Attribute> attrs = catalogManager->getTableAttributes(tablename);
    // 确认属性合法
    for (auto &c : compare)
    {
        bool validAttr = false;
        for (auto &a : attrs)
        {
            if (a.attrName == c.attrName)
            {
                validAttr = true;
                if (a.attrType != c.data.type)
                {
                    // 有可能输入条件中的float被当作int
                    if (a.attrType == 0 && c.data.type == -1)
                    {
                        c.data.type = 0;
                        c.data.dataf = c.data.datai;
                    }
                    else if (a.attrType < 1 && c.data.type < 1)
                    { // 两个都是字符串不管长度
                        throw logic_error("Error: Invalid value of " + c.attrName + ".");
                    }
                }
                break;
            }
        }
        if (!validAttr)
            throw logic_error("Error: Invalid attribute " + c.attrName + ".");
    }
#ifdef DEBUG
    for (auto &c : compare)
    {
        cout << c.attrName << ' ';
        switch (c.data.type)
        {
        case -1:
            cout << c.data.datai << endl;
        case 0:
            cout << c.data.dataf << endl;
        default:
            cout << c.data.datas << endl;
        }
    }
#endif
    vector<Tuple> records;
    if (compare.empty())
        records = recordManager->selectRecord(tablename);
    else
        records = recordManager->selectRecord(tablename, compare);

    return records;
}

void API::deleteRecord(const string &tablename, vector<Compare> &compare)
{
    if (!catalogManager->isTableExist(tablename))
        throw runtime_error("Error: Table " + tablename + " doesn't exist.");
    vector<Attribute> attrs = catalogManager->getTableAttributes(tablename);
    // 确认属性合法
    for (auto &c : compare)
    {
        bool validAttr = false;
        for (auto &a : attrs)
        {
            if (a.attrName == c.attrName)
            {
                validAttr = true;
                if (a.attrType != c.data.type)
                {
                    // 有可能输入条件中的float被当作int
                    if (a.attrType == 0 && c.data.type == -1)
                    {
                        c.data.type = 0;
                        c.data.dataf = c.data.datai;
                    }
                    else if (a.attrType < 1 && c.data.type < 1)
                    {
                        throw logic_error("Error: Invalid value of " + c.attrName + ".");
                    }
                }
                break;
            }
        }
        if (!validAttr)
            throw logic_error("Error: Invalid attribute " + c.attrName + ".");
    }

    if (compare.empty())
        recordManager->deleteRecord(tablename);
    else
        recordManager->deleteRecord(tablename, compare);
}

bool API::insertRecord(const string &tablename, const Tuple &tuple)
{
    if (!catalogManager->isTableExist(tablename))
        throw runtime_error("Error: Table " + tablename + " doesn't exist.");
    return recordManager->insertRecord(tablename, tuple);
}

bool API::createTable(const string &tablename, const vector<Attribute> &attribute, const string &primary)
{
    if (!catalogManager->isTableExist(tablename))
    {
        Table table(tablename, attribute, primary);
        catalogManager->createTable(table);
        recordManager->createTableFile(tablename);
        return true;
    }
    else
    {
        throw logic_error("Error: Table " + tablename + " alreadey existed.");
    }
}

bool API::dropTable(const string &tablename)
{
    if (catalogManager->isTableExist(tablename))
    {
        catalogManager->deleteTable(tablename);
        recordManager->dropTableFile(tablename);
        return true;
    }
    else
    {
        throw runtime_error("Error: Table " + tablename + " doesn't exist.");
    }
}

bool API::createIndex(const string &tablename, const string &indexname, const string &attrname)
{
    if (!catalogManager->isTableExist(tablename))
        throw runtime_error("Error: Table " + tablename + " doesn't exist.");

    string table = tablename, attr = attrname;
    if (!catalogManager->isIndexExist(indexname))
    {
        Index index(indexname, tablename, attrname);
        catalogManager->createIndex(index);
        indexManager->createIndex(tablename, indexname, attrname);
        return true;
    }
    else
    {
        throw runtime_error("Error: Index " + indexname + " already existed.");
    }
}

bool API::dropIndex(const string &indexname)
{
    if (catalogManager->isIndexExist(indexname))
    {
        string tablename = catalogManager->getIndex(indexname).tableName;
        indexManager->dropIndex(tablename, indexname);
        catalogManager->dropIndex(indexname);
        return true;
    }
    else
    {
        throw runtime_error("Error: Index " + indexname + " doesn't existed.");
    }
}

API::~API()
{
    delete bufferPoolManager;
    delete indexManager;
    delete recordManager;
    delete catalogManager;
}
