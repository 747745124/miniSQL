#ifndef TABLE_H
#define TABLE_H

#include <string>
#include <vector>
#include "index.h"
#include "attribute.h"
#include "tuple.h"

using namespace std;

class Table {
public:
    string tableName;             //表名
    string primaryKey;            //主键名
    vector<Attribute> attributes; //以vector方式存放字段
    vector<Index> indexes;        //以vector方式存放表上的索引
    vector<Tuple> tuples;         //

    Table() = default;

    //创建表格时的构造方法
    Table(string tableName, vector<Attribute> attributes, string primaryKey);

    //读取文件中表格信息的构造方法
    Table(string tableName, vector<Attribute> attributes, vector<Index> indexes, string primaryKey);

    int getTupleLength();
};

#endif