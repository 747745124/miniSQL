//
// Created by 93209 on 2021/6/16.
//

#include "CatalogManager.h"
#include <fstream>

const string CatalogManager::fileName = "./data/tables.catalog";

void CatalogManager::readCatalog()
{
    int tableCount = 0, attrCount = 0, indexCount = 0;

    ifstream in(fileName, ios::in);
    if (!in.is_open())
    {
        ofstream out(fileName, ios::out);
        out << 0 << endl;
    }
    else
    {
        in >> tableCount;
        for (int i = 0; i < tableCount; i++)
        {
            Table table;
            in >> table.tableName;
            in >> attrCount;
            for (int j = 0; j < attrCount; j++)
            {
                Attribute attr;
                in >> attr.attrName >> attr.attrType >> attr.length >> attr.isUnique;
                table.attributes.push_back(attr);
            }
            in >> indexCount;
            for (int j = 0; j < indexCount; j++)
            {
                Index index;
                in >> index.indexName >> index.tableName >> index.attrName;
                table.indexes.push_back(index);
                indexes[index.indexName] = index;
            }
            tables[table.tableName] = table;
        }
    }
}

void CatalogManager::writeCatalog()
{
    int tableCount = tables.size();

    ofstream out(fileName, ios::out);
    if (!out.is_open())
    {
        exit(-114);
    }
    else
    {
        out << tableCount << endl;
        for (auto &table : tables)
        {
            auto &t = table.second;
            out << t.tableName << endl;
            out << t.attributes.size() << endl;
            for (auto &a : t.attributes)
            {
                out << a.attrName << endl;
                out << a.attrType << endl;
                out << a.length << endl;
                out << a.isUnique << endl;
            }
            out << t.indexes.size() << endl;
            for (auto &i : t.indexes)
            {
                out << i.indexName << endl;
                out << i.tableName << endl;
                out << i.attrName << endl;
            }
        }
    }
}

bool CatalogManager::createTable(Table &newTable)
{
    if (tables.find(newTable.tableName) != tables.end())
        return false;
    tables[newTable.tableName] = newTable;
    writeCatalog();
    return true;
}

bool CatalogManager::deleteTable(const string &tableName)
{
    if (tables.find(tableName) == tables.end())
        return false;
    auto &t = tables[tableName];
    for (auto &i : t.indexes)
    {
        indexManager->dropIndex(tableName, i.indexName);
        indexes.erase(i.indexName);
    }
    tables.erase(tableName);
    writeCatalog();
    return true;
}

bool CatalogManager::createIndex(Index &newIndex)
{
    if (tables.find(newIndex.tableName) == tables.end())
        return false;
    if (indexes.find(newIndex.indexName) != indexes.end())
        return false;
    indexes[newIndex.indexName] = newIndex;
    tables[newIndex.tableName].indexes.push_back(newIndex);
    writeCatalog();
    return true;
}

bool CatalogManager::dropIndex(const string &indexName)
{
    if (indexes.find(indexName) == indexes.end())
        return false;
    auto &i = indexes[indexName];
    auto &t = tables[i.tableName];

    for (auto it = t.indexes.begin(); it != t.indexes.end(); it++)
    {
        if (it->indexName == indexName)
        {
            t.indexes.erase(it);
            break;
        }
    }
    indexes.erase(indexName);
    writeCatalog();
    return true;
}

bool CatalogManager::isPrimaryKey(string &tableName, string &attrName)
{
    if (tables.find(tableName) == tables.end())
        return false;
    return (tables[tableName].primaryKey == attrName);
}

bool CatalogManager::isUniqueKey(string &tableName, string &attrName)
{
    if (tables.find(tableName) == tables.end())
        return false;
    auto &t = tables[tableName];
    for (auto &a : t.attributes)
    {
        if (a.attrName == attrName)
        {
            return a.isUnique;
        }
    }
    return false;
}

bool CatalogManager::isIndexKey(const string &tableName, const string &attrName)
{
    if (tables.find(tableName) == tables.end())
        return false;
    auto &t = tables[tableName];

    for (auto &i : t.indexes)
    {
        if (i.attrName == attrName)
            return true;
    }
    return false;
}

CatalogManager::CatalogManager()
{
    readCatalog();
}

CatalogManager::~CatalogManager()
{
    writeCatalog();
}
