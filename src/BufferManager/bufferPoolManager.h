#pragma once

#include <list>
#include <unordered_map>
#include <string.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <exception>
#include "lruReplacer.h"
#include "page.h"

using namespace std;

class BufferPoolManager
{
public:
    //bufferCapacity是pages数组的大小
    BufferPoolManager(int bufferCapacity)
    {
        this->bufferCapacity = bufferCapacity;
        pages = new Page[bufferCapacity];
        replacer = new LruReplacer(bufferCapacity);
        for (int i = 0; i < bufferCapacity; ++i)
        {
            freeList.emplace_back(static_cast<int>(i));
        }
    }

    ~BufferPoolManager()
    {
        flushAllPages();
        delete replacer;
        delete[] pages;
    }

    //找到合适的替换位置,如果成功则返回true,值记录在*frameId_
    //否则返回false
    bool getReplace(frameId_t &frameId_)
    {
        if (!freeList.empty())
        {
            frameId_ = freeList.front();
            //std::cout<<"llll"<<frameId_<<std::endl;
            freeList.pop_front();
            return true;
        }
        if (replacer->victim(frameId_))
        {
            if (pages[frameId_].getDirty())
            {
                //写入磁盘
                flushPage(pages[frameId_].getPageId(), pages[frameId_].getFileName());
            }
            pages[frameId_].setDirty(false);
            pages[frameId_].setMemory();
            pageMap[pages[frameId_].getFileName()].erase(pages[frameId_].getPageId());
            return true;
        }
        return false;
    }

    //返回bufferpool中储存page的pages数组的指针
    Page *getPages() { return pages; }

    //返回bufferpool中pages数组的大小
    int getBufferCapacity() { return bufferCapacity; }

    //访问pageId页，返回该page的指针
    Page *fetchPage(pageId_t page_id, fileName_t file_name)
    {
        frameId_t frameId_ = -1;
        //请求获取的页没有被写过
        if (!pageExist(file_name, page_id))
        {
            return nullptr;
        }
        //请求获取的页在bufferpool中
        if (pageMap[file_name].find(page_id) != pageMap[file_name].end())
        {
            frameId_ = pageMap[file_name][page_id];
            return &pages[frameId_];
        }
        //请求获取的页存在，但不在bufferpool中，如果bufferpool容量足够就读入，否则返回nullptr
        if (getReplace(frameId_))
        {

            pageMap[file_name][page_id] = frameId_;
            pages[frameId_].setFileName(file_name);
            pages[frameId_].setPageId(page_id);
            //读入内容
            //1.打开文件
            openFile(file_name);
            //2.读文件
            int offset = static_cast<int>(page_id) * PAGESIZE;
            fileIo.seekp(offset);
            fileIo.read(pages[pageMap[file_name][page_id]].getContent(), PAGESIZE);
            if (fileIo.bad())
            {
                throw runtime_error("I/O error while writing");
            }
            //所读文件片段小于PAGESIZE（提前截止）
            int readCount = fileIo.gcount();
            if (readCount < PAGESIZE)
            {
                //    std::cout<<"read less than a page"<<std::endl;
                fileIo.clear();
                memset(pages[pageMap[file_name][page_id]].getContent() + readCount, 0, PAGESIZE - readCount);
            }
            fileIo.close();

            //刚读入的新page的pinCount=0,要进入lru的“可替换链”，所以使用unpin
            replacer->unpin(frameId_);

            return &pages[frameId_];
        }
        return nullptr;
    }

    //为pageId页加pin(pinCount++)
    bool pinPage(pageId_t page_id, fileName_t file_name)
    {
        auto itor = pageMap[file_name].find(page_id);
        if (itor == pageMap[file_name].end())
        {
            return false;
        }
        frameId_t frameId = itor->second;
        if (pages[frameId].getPinCount() == 0)
        {
            replacer->pin(frameId);
        }
        pages[frameId].setPinCount(pages[frameId].getPinCount() + 1);
        return true;
    }
    //为pageId页解除pin（pinCount--),不代表pinCount=0
    bool unpinPage(pageId_t page_id, fileName_t file_name)
    {
        auto itor = pageMap[file_name].find(page_id);
        if (itor == pageMap[file_name].end())
        {
            return false;
        }
        frameId_t frameId = itor->second;
        if (pages[frameId].getPinCount() > 0)
        {
            pages[frameId].setPinCount(pages[frameId].getPinCount() - 1);
        }
        if (pages[frameId].getPinCount() == 0)
        {
            replacer->unpin(frameId);
        }
        return true;
    }
    //将pageId页写入磁盘
    bool flushPage(pageId_t page_id, fileName_t file_name)
    {
        auto itor = pageMap[file_name].find(page_id);
        if (itor == pageMap[file_name].end())
        {
            return false;
        }
        frameId_t frameId = itor->second;
        //写入磁盘
        //1.打开文件
        openFile(file_name);
        //2.写入文件
        int offset = static_cast<int>(page_id) * PAGESIZE;
        fileIo.seekp(offset);
        fileIo.write(pages[pageMap[file_name][page_id]].getContent(), PAGESIZE);
        if (fileIo.bad())
        {
            throw runtime_error("I/O error while writing");
        }
        fileIo.flush();
        fileIo.close();
        pages[frameId].setDirty(false);
        return true;
    }
    //在bufferpool中新建一个page,由filePageIdMap分配的pageId
    Page *newPage(fileName_t file_name, pageId_t &page_id_)
    {
        frameId_t frameId_;
        if (getReplace(frameId_))
        {

            if (filePageIdMap.find(file_name) == filePageIdMap.end())
            {
                filePageIdMap[file_name] = 0;
            }
            else
            {
                filePageIdMap[file_name]++;
            }
            pageId_t pageId = filePageIdMap[file_name];
            page_id_ = pageId;

            pageMap[file_name][pageId] = frameId_;
            pages[frameId_].setPageId(pageId);
            pages[frameId_].setFileName(file_name);
            pages[frameId_].setDirty(true);

            //刚生成的新page的pinCount=0,要进入lru的“可替换链”，所以使用unpin
            replacer->unpin(frameId_);
            return &pages[frameId_];
        }
        return nullptr;
    }
    //判断给定文件名下的给定page是否写过
    bool pageExist(fileName_t file_name, pageId_t page_id)
    {
        if (filePageIdMap.find(file_name) == filePageIdMap.end())
        {
            int size = getFileSize(file_name);
            if (size < 1)
            {
                return false;
            }
            else
            {
                filePageIdMap[file_name] = size - 1;
                return true;
            }
        }
        else if (page_id > filePageIdMap[file_name])
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    //将bufferpool中所有page写入磁盘
    void flushAllPages()
    {
        for (int i = 0; i < bufferCapacity; ++i)
        {
            if (pages[i].getPageId() == -1)
            {
                continue;
            }
            else
            {
                flushPage(pages[i].getPageId(), pages[i].getFileName());
            }
        }
    }
    //检查文件名格式，打开文件
    void openFile(fileName_t file_name)
    {
        //1、格式检查
        std::string::size_type a = file_name.rfind('.');
        if (a == std::string::npos)
        {
            std::cout << "the file :" << file_name << "is wrong" << std::endl;
            return;
        }
        //2.打开文件
        fileIo.open(file_name, std::ios::binary | std::ios::in | std::ios::out);
        if (!fileIo.is_open())
        {
            fileIo.clear();
            fileIo.open(file_name, std::ios::binary | std::ios::trunc | std::ios::out);
            fileIo.close();
            fileIo.open(file_name, std::ios::binary | std::ios::in | std::ios::out);
            if (!fileIo.is_open())
            {
                std::cout << "can't open " << file_name << std::endl;
                return;
            }
        }
    }
    //删除文件并删除该文件下bufferpool中的所有页
    void deleteFileAndPages(fileName_t file_name)
    {
        //   cout << "deleteFileAndPages: " << file_name << endl;
        if (pageMap.find(file_name) == pageMap.end())
        { //该文件不存在
            // 不存在正好
            // throw runtime_error(file_name + " doesn't exist.");
        }
        else
        { //该文件存在（则至少有一个page）
            //   std::cout<<"have a file called"<<file_name<<std::endl;
            pageId_t maxPageId = filePageIdMap[file_name];
            //   std::cout<<"maxPageId: "<<maxPageId<<file_name<<std::endl;
            for (pageId_t i = 0; i <= maxPageId; ++i)
            {
                frameId_t frame_id = pageMap[file_name][i];
                //删除replacer中该frame_id对应的数据，作用等效于在replacer中使用pin
                replacer->pin(frame_id);
                filePageIdMap.erase(file_name);
                pageMap.erase(file_name);
                pages[frame_id].setPageId(-1);
                pages[frame_id].setDirty(false);
                pages[frame_id].setFileName("");
                pages[frame_id].setPinCount(0);
                pages[frame_id].setMemory();
                freeList.emplace_back(frame_id);
            }
        }
        remove(file_name.c_str());
    }
    int getFileSize(const std::string &file_name)
    {
        int size = -1;
        std::ifstream in(file_name);
        in.seekg(0, std::ios::end); //设置文件指针到文件流的尾部
        size = in.tellg() / 4096;

        //  std::cout<<size<<std::endl;
        return size;
    }
    int getFileLastPageId(fileName_t file_name)
    {
        if (filePageIdMap.find(file_name) == filePageIdMap.end())
        {
            int size = getFileSize(file_name);
            if (size < 1)
            {
                return -1;
            }
            else
            {
                filePageIdMap[file_name] = size - 1;
                return size - 1;
            }
        }
        else
        {
            return filePageIdMap[file_name];
        }
    }

private:
    //bufferpool的大小
    int bufferCapacity;
    //bufferpool中用来储存page的数组
    Page *pages;
    //使用map关联bufferpool的<文件名,<frameId,pageId>>
    std::unordered_map<fileName_t, std::unordered_map<pageId_t, frameId_t>> pageMap;
    //使用map记录每个文件中下一次分配的pageId
    std::unordered_map<fileName_t, pageId_t> filePageIdMap;
    //lruReplacer
    LruReplacer *replacer;
    //记录bufferpool中的空闲位置
    std::list<frameId_t> freeList;
    //文件流
    std::fstream fileIo;
};
