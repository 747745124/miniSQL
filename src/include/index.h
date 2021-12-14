#ifndef INDEX_H
#define INDEX_H

#include <string>
#include <vector>
using namespace std;
class Index
{
public:
    string indexName; //索引名，唯一标记索引
    string tableName; //表名
    string attrName;  //属性名
    Index() = default;
    Index(string indexName, string tableName, string attrName)
    {
        this->indexName = indexName;
        this->tableName = tableName;
        this->attrName = attrName;
    }
    ~Index() = default;
};
#endif