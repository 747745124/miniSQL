#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include <string>
using namespace std;

class Attribute
{
public:
    Attribute() = default;
    Attribute(string attrName, int attrType, int length, bool isUnique) {
        this->attrName = attrName;
        this->attrType = attrType;
        this->length = length;
        this->isUnique = isUnique;
    }
    ~Attribute() = default;
    string attrName; //字段名称
    int attrType;    //-1:int 0:float 1-255:char(n)
    int length;      //字段长度
    bool isUnique;   //是否Unique
};
#endif