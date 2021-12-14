#include"RecordFunction.h"

string Datatostring(const Data &a, Attribute type) //把Data類型轉換為string, 且位數不足時補充空格在左方
{
    switch (type.attrType) {
        case (-1):
            return blank(a.datai, 16);
            break;
        case (0):
            return blank(a.dataf, 16);
            break;
        default:
            return blank(a.datas, type.length);
    }
}

bool isCompare(const Data &a, const Data &b)        //比較是否相等
{
    switch (b.type) {
        case (-1):
            if (a.datai != b.dataf) return false;
            break;
        case (0):
            if (a.dataf != b.dataf) return false;
            break;
        default:
            if (a.datas != b.datas) return false;
            break;
    }
    return true;
}

bool isMatch(const Data &data, const Compare &compare) {

    switch (data.type) {
        case (-1):
            if (!inRange(data.datai, compare.data.datai, compare.type)) return false;
            break;
        case (0):
            if (!inRange((double) (float) data.dataf, compare.data.dataf, compare.type)) return false;
            break;
        default:
            if (!inRange(data.datas, compare.data.datas, compare.type)) return false;
            break;
    }
    return true;
}

Tuple readTuple(std::string str, std::vector<Attribute> attr) //從字符串中根據屬性提取成Tuple
{
    Tuple tuple;
    vector<Data> data;
    int k = 0;
    for (int i = 0; i < attr.size(); i++) {
        string temp;
        Data t;
        temp = "";
        t.type = attr[i].attrType;  //獲得數據類型

        int j;
        int num = attr[i].length + k;
        for (j = k; j < num && str[j] == ' '; j++); //去除填充的空格

        for (k = j; k < num; k++)
            temp = temp + str[k];

        stringstream ss;   //轉換成Data類
        switch (t.type) {
            case (-1):
                ss << temp;
                ss >> t.datai;

                break;
            case (0):
                ss << temp;
                ss >> t.dataf;
                break;
            default:
                t.datas = temp;
        }
        tuple.addData(t);
    }
    return tuple;
}

