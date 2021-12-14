//
// Created by 93209 on 2021/6/3.
//

#ifndef MINISQL_BUFFERMANAGERDEMO_H
#define MINISQL_BUFFERMANAGERDEMO_H

#include "../basic.h"
#include <unordered_map>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <map>
using namespace std;

struct PageDemo {
    Byte data[4096]{};
};

class BufferManagerDemo {
public:
    int fetchCount = 0;
    unordered_map<PageIndex, PageDemo> pages;
    PageDemo* fetchPage(const string &fileName, PageIndex index) {
        fetchCount++;
        if (pages.find(index) == pages.end()) {
            return nullptr;
        }
        return &pages[index];
    }
    PageIndex newPage(const string &fileName) {
        int maxIndex = -1;
        for (auto &i : pages) {
            maxIndex = max(maxIndex, i.first);
        }
        maxIndex++;
        pages[maxIndex] = PageDemo();
        return maxIndex;
    }
    bool flushPage(const string &fileName, PageIndex index) {
        return true;
    }
    void deletePage(const string &fileName, PageIndex index) {
        return;
    }

};


#endif //MINISQL_BUFFERMANAGERDEMO_H
