#ifndef _RECORDFUNCTION_H_
#define _RECORDFUNCTION_H_

#include "../BufferManager/bufferPoolManager.h"
#include "../basic.h"
#include <iostream>
#include <vector>
#include<sstream>

template<class T>
string toString(T a) {
    stringstream ss;
    ss << a;
    return ss.str();
}

template<class T>
string blank(T a, int num)  //用於給Data填充空格並轉換類型的模版函數
{
    stringstream ss;
    string str;
    ss << a;

    str = ss.str(); //轉換類型為string

    return string(num - str.length(), ' ') + str;

//    while (str.length() < num) {
//        str = " " + str;
//    }
//    return str;
}

string Datatostring(const Data &a, Attribute type); //把Data類型轉換為string, 且位數不足時補充空格在左方

bool isCompare(const Data &a, const Data &b); //比較是否相等

template<class T>
bool inRange(T a, T b, CompareType condition) {
    stringstream ss;
    string t1, t2;
    switch (condition) {
        case CompareType::E:
            if (a == b) return true;
            break;
//        ss << a; ss >> t1;
//        ss.clear();
//        ss << b; ss >> t2;
//        if (!strcmp(t1.c_str(),t2.c_str())) return true; break; //==
        case CompareType::G:
            if (a > b) return true;
            break; //>
        case CompareType::L:
            if (a < b) return true;
            break; //<
        case CompareType::GE:
            if (a >= b) return true;
            break; //>=
        case CompareType::LE:
            if (a <= b) return true;
            break; // <=
        case CompareType::NE:
            if (a != b) return true;
            break; // <>
    }
    return false;
}

bool isMatch(const Data &data, const Compare &compare);

Tuple readTuple(std::string str, std::vector<Attribute> attr); //從字符串中根據屬性提取成Tuple

#endif