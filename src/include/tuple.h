#ifndef TUPLE_H
#define TUPLE_H

#include <string>
#include <vector>

#include "data.h"

class Tuple {
   public:
    Tuple() : deleteTuple(false){};  //構造函數

    Tuple(const Tuple& tupleIn)      //拷貝函數
    {
        deleteTuple = false;
        for (int i = 0; i < tupleIn.data.size(); i++)
        {
            data.push_back(tupleIn.data[i]);
        }
    }

    //返回值: 0:插入失敗 1:插入成功
    //功能: 新增數據到tuple中
    int addData(Data newdata)
    {
        if (newdata.type > 255 || newdata.type < -1 || deleteTuple) return 0;
        data.push_back(newdata);
        return 1;
    }

    //返回值: 數據個數,如果被標記刪除則返回0
    int getSize() const 
    {
        if (!deleteTuple) return (int)data.size();
        else return 0;
    }

    //參數表: tuple
    //返回值: tuple中的數據
    std::vector<Data> getData() const
    {
        return data;
    }

    //功能: 標記該tuple已被刪除
    void deleteData()
    {
        deleteTuple = true;
    }

    Data getData(int i) {
        return data.at(i);
    }

   private:
    std::vector<Data> data;
    bool deleteTuple;  // true:己被刪除 false:仍在
};

#endif // TUPLE_H