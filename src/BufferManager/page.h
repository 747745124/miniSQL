#pragma once

#include <cstring>
#include <iostream>
#include <string>

const int PAGESIZE = 4096;
typedef int pageId_t;
typedef std::string fileName_t;

class Page {
public:
    Page() { setMemory(); };

    ~Page() = default;

    //返回pageContent[PAGESIZE]首地址
    char *getContent() { return pageContent; }

    //返回该page的pageId
    pageId_t getPageId() { return pageId; }

    //返回该page的pinCount的数量
    int getPinCount() { return pinCount; }

    //设置持有该page的线程数
    void setPinCount(int s) {
        pinCount = s;
    }

    //返回dirty位，即该page是否被修改过
    bool getDirty() { return dirty; }

    //设置dirty位
    void setDirty(bool s) {
        dirty = s;
    }

    //返回该page所属文件的文件名
    fileName_t getFileName() { return fileName; }

    //设置该page所属文件的文件名
    void setFileName(fileName_t s) {
        this->fileName = s;
    }

    //返回该page的pageId
    //pageId_t getPageId(){ return pageId; }
    //设置该page的pageId
    void setPageId(pageId_t pageId) {
        this->pageId = pageId;
    }

    //将数组空间用0填充
    void setMemory() { memset(pageContent, 0, PAGESIZE); }

private:
    //page中记录数据所用的字符数组
    char pageContent[PAGESIZE];
    //文件名
    std::string fileName = "";
    //pageId初始为0
    pageId_t pageId = -1;
    //page是否被修改过
    bool dirty = false;
    //pin的状态
    int pinCount = 0;

};
