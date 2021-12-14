#ifndef CATALOGMANAGER_H
#define CATALOGMANAGER_H

/*Catalog Manager负责管理数据库的所有模式信息，包括：
1.	数据库中所有表的定义信息，包括表的名称、表中字段（列）数、主键、定义在该表上的索引。
2.	表中每个字段的定义信息，包括字段类型、是否唯一等。
3.	数据库中所有索引的定义，包括所属表、索引建立在那个字段上等。
Catalog Manager还必需提供访问及操作上述信息的接口，供Interpreter和API模块使用。
*/
#include "../include/attribute.h"
#include "../include/index.h"
#include "../include/table.h"
#include "../IndexManager/IndexManager.h"
#include <string>
#include <vector>
#include <map>

using namespace std;

class IndexManager;

class CatalogManager
{
private:
    IndexManager *indexManager;
    //用于存储tables和indexes的信息
    map<string, Table> tables;
    map<string, Index> indexes;
    //存储catalog文件的路径
    static const string fileName;

public:
    //与indexManager进行绑定
    void setPointer(IndexManager *index)
    {
        indexManager = index;
    }

    CatalogManager();

    ~CatalogManager();
    //catalog文件读写
    void readCatalog();

    void writeCatalog();

    /*
    创建表
    Constructor:Table(string tableName, vector<Attribute> attributes, string primaryKey);
    */
    bool createTable(Table &newTable);

    /*
    根据表名删除表
    */
    bool deleteTable(const string &tableName);

    /*
    创建index
    Constructor:Index(string indexName, string tableName, string attrName);
    */
    bool createIndex(Index &newIndex);

    /*
    根据indexName删除Index
    */
    bool dropIndex(const string &indexName);

    /*
    根据tableName返回table
    */
    Table getTable(const string &tableName)
    {
        return tables[tableName];
    }

    /*
    根据indexName返回index
    */
    Index getIndex(const string &indexName)
    {
        return indexes[indexName];
    }

    /*
    根据tableName返回Table primary key
    */
    string getPrimaryKey(string &tableName)
    {
        return getTable(tableName).primaryKey;
    }

    /*
    根据tableName返回Table tuple length
    */
    int getTupleLength(string &tableName)
    {
        return getTable(tableName).getTupleLength();
    }

    /*
    根据tableName返回Table attributes个数
    */
    int getTableAttrNum(string &tableName)
    {
        return getTable(tableName).attributes.size();
    }

    /*
    根据tableName返回Table的所有attributes
    */
    vector<Attribute> getTableAttributes(const string &tableName)
    {
        return getTable(tableName).attributes;
    }

    /*
    根据tableName返回Table的所有index
    */
    vector<Index> getIndexes(const string &tableName)
    {
        return getTable(tableName).indexes;
    }

    /*
    给定表名,attributeName，判断是否是primary key
    */
    bool isPrimaryKey(string &tableName, string &attrName);

    /*
    给定表名,attribute Name，判断是否是unique key
    */
    bool isUniqueKey(string &tableName, string &attrName);

    /*
    给定表名,attribute Name，判断是否是index key
    */
    bool isIndexKey(const string &tableName, const string &attrName);

    /*
    给定表名，判断是否存在
    */
    bool isTableExist(const string &tableName)
    {
        if (tables.find(tableName) == tables.end())
        { //if not found, return false
            return false;
        }
        return true;
    }

    /*给定indexName，判断是否存在*/
    bool isIndexExist(const string &indexName)
    {
        if (indexes.find(indexName) == indexes.end())
        { //if not found, return false
            return false;
        }
        return true;
    }

    /*给定Attribute，判断是否存在*/
    bool isAttrExist(const string &tableName, const string &attrName)
    {
        for (auto &attr : getTableAttributes(tableName))
        {
            if (attr.attrName == (attrName))
            {
                return true;
            }
        }
        return false;
    }
};

#endif