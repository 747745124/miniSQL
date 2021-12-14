#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <algorithm>
#include <iostream>
#include <vector>

#include "../basic.h"
#include "../API/API.h"

using namespace std;

class Interpreter
{
#ifdef DEBUG
public:
#else
private:
#endif
    //SQL保留字的常量
    static const std::string QUIT;
    static const std::string CREATE;
    static const std::string DROP;
    static const std::string INSERT;
    static const std::string DELETE;
    static const std::string SELECT;
    static const std::string EXECFILE;

    static const std::string INDEX;
    static const std::string TABLE;
    static const std::string VALUES;

    static const std::string INTO;
    static const std::string ON;
    static const std::string FROM;
    static const std::string UNIQUE;
    static const std::string PRIMARY;
    static const std::string KEY;

    static const std::string FLOAT;
    static const std::string INT;
    static const std::string CHAR;
    //输入的query
    std::string query;
    //用于存储关键字的vector
    std::vector<std::string> words;
    //用于检查关键字是否属于string
    std::vector<int> isString;
    //api用于调取指令
    API *api;
    //execFile指令
    void runFile();
    //大小写转换
    void toLower(string &str)
    {
        transform(str.begin(), str.end(), str.begin(),
                  [](char c) -> char
                  { return tolower(c); });
    }
    //是否为SQL关键字
    bool isKeyWord(const string &word)
    {
        string tmp = word;
        toLower(tmp);
        return tmp == QUIT || tmp == CREATE || tmp == DROP || tmp == INSERT ||
               tmp == DELETE || tmp == SELECT || tmp == INDEX || tmp == TABLE ||
               tmp == VALUES || tmp == INTO || tmp == ON || tmp == FROM ||
               tmp == UNIQUE || tmp == PRIMARY || tmp == KEY || tmp == FLOAT ||
               tmp == INT || tmp == CHAR;
    }
    //interpreter封装后的接口，包含语法检查+错误检查+获取参数+调取api执行
    void createTable();
    void createIndex();
    void dropTable();
    void dropIndex();
    void insert();
    //用于获取WHERE语句的相关信息
    void getCompares(vector<Compare> &compares, int i);
    //用于SELECT语句输出结果
    void showTable(const vector<Tuple> &tuples, const vector<Attribute> &attrs);
    //用于整理SELECT语句输出结果的格式
    string format(const string &str, int length, int isNum);

public:
    Interpreter();
    ~Interpreter();
    //query关键词拆分到words中，关键字isString判断，大小写转换
    void setQuery(std::string s);
    //执行set好的query，返回-1表明没有关键字，0表示退出，1表示执行了非quit的语句
    int runQuery();
};

#endif