#ifndef DATA_H
#define DATA_H

#include <string>

class Data {
public:
    int type; // -1:int 0:float 1-255:char(n)
    int datai;
    double dataf;
    std::string datas;

    Data() { type = -2; }

    Data(int data) {
        type = -1;
        datai = data;
    }

    Data(double data) {
        type = 0;
        dataf = data;
    }

    Data(std::string data) {
        type = data.length();
        datas = data;
    }

    int getSize() {
        if (type == -1) return sizeof(int);
        else if (type == 0) return sizeof(double);
        else return datas.length();
    }
};

#endif // DATA_H