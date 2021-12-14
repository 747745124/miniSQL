#ifndef COMPARE_H
#define COMPARE_H

#include <string>
#include <vector>

#include "data.h"

enum class CompareType {
    // equal, greater, less, g&e, l&e, undefined
    E, G, L, GE, LE, NE, UD
};

class Compare {
public:
    std::string attrName;
    CompareType type;
    Data data;

    Compare() : type(CompareType::UD) {}

    Compare(std::string attrName, CompareType type, Data data) {
        this->attrName = attrName;
        this->type = type;
        this->data = data;
    }
};

#endif // COMPARE_H