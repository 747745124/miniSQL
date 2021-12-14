#ifndef _RECORD_H_
#define _RECORD_H_

#include "../IndexManager/IndexManager.h"
#include "../BufferManager/bufferPoolManager.h"
#include "../CatalogManager/CatalogManager.h"
#include "RecordFunction.h"
#include <iostream>
#include <string>
#include <stdio.h>
#include <set>
#include <unordered_map>

using namespace std;

class IndexManager;
class BufferPoolManager;
class CatalogManager;

class IndexManager;
class BufferPoolManager;
class CatalogManager;

class RecordManager
{
public:
    void setPointer(IndexManager* indexmanager, CatalogManager* catalogmanager, BufferPoolManager* buffermanager);

    void createTableFile(const string &tablename);
    // 參數表: 表格名
    // 返回值: 無
    // 功能: 新增表格文件

    void dropTableFile(const string &tablename);
    // 參數表: 表格名
    // 返回值: 無
    // 功能: 刪除表格文件

    int insertRecord(const string &tablename, const Tuple &tuple_in);
    // 參數表: 表格名, 元組
    // 返回值: 1: 插入成功 0:1插入非法
    // 功能: 插入記錄

    void deleteRecord(const string &tablename);
    // 參數表: 表格名
    // 返回值: 無
    // 功能: 刪除表中所有tuple,但不刪除表

    void deleteRecord(const string &tablename, const vector<Compare> &compare);
    // 參數表: 表格名, Class Compare
    // 返回值: 無
    // 功能: 刪除符合where中條件的tuple

    void deleteRecord(const string &tablename, const Position &pos, const int &number);
    // 參數表: 表格名,Class Position, 刪除tuple的數量
    // 返回值: 無
    // 功能: 刪除指定pageid中指定的tuple

    vector<Tuple> selectRecord(const string &tablename);
    // 參數表: 表格名
    // 返回值: tuples
    // 功能: 返回所有記錄

    vector<Tuple> selectRecord(const string &tablename, const vector<Compare> &compare);
    // 參數表: 表格名, Class Compare
    // 返回值: tuples
    // 功能: 返回符合where中條件的記錄

    vector<Tuple> selectRecord(const string &tablename, const Position &pos, const int number = 1);
    // 參數表: 表格名, Class Position, tuple的數量
    // 返回值: tuples
    // 功能:  返回指定pageid中指定的tuple
    void getIndexData(const string &tablename, int attrIndex, vector<Data> &datas, vector<Position> &positions);

private:
    bool check(const string &tablename, const Tuple &tuple_in); //檢查tuple是否合法
    int getpageid(const std::string& file, int length);//取得page_id
    IndexManager* indexmanager;
    CatalogManager *catalogmanager;
    BufferPoolManager *buffermanager;
    unordered_map<string,set<pair<string, string>>> uniqure;
};

#endif
