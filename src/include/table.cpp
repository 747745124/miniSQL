//
// Created by 93209 on 2021/6/16.
//


#include "table.h"


Table::Table(string tableName, vector<Attribute> attributes, string primaryKey) {
    this->tableName = tableName;
    this->attributes = attributes;
    this->primaryKey = primaryKey;
}

Table::Table(string tableName, vector<Attribute> attributes, vector<Index> indexes,
             string primaryKey) {
    this->tableName = tableName;
    this->attributes = attributes;
    this->indexes = indexes;
    this->primaryKey = primaryKey;
}

int Table::getTupleLength() {
    int res = 0;
    for (auto &a : attributes)
        res += a.length;
    return res;
}
