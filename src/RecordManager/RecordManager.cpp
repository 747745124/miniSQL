#include "RecordManager.h"
#ifdef TIME
#include <chrono>
#include <iomanip>
using namespace std;
using namespace chrono;
#endif

void RecordManager::setPointer(IndexManager *indexmanager, CatalogManager *catalogmanager, BufferPoolManager *buffermanager)
{
    this->indexmanager = indexmanager;
    this->catalogmanager = catalogmanager;
    this->buffermanager = buffermanager;
}

void RecordManager::createTableFile(const string &tablename)
{
    FILE *fp;
    string file = "./data/" + tablename + ".txt"; //暫定
    fp = fopen(file.c_str(), "w+");
    fclose(fp);
}

void RecordManager::dropTableFile(const string &tablename)
{
    uniqure.erase(tablename);
    string file = "./data/" + tablename + ".txt"; //暫定
    // remove(file.c_str());
    buffermanager->deleteFileAndPages(file);
}

int RecordManager::insertRecord(const string &tablename, const Tuple &tuple_in)
{
#ifdef TIME
    auto startTime = system_clock::now();
#endif
    string file = "./data/" + tablename + ".txt";
    string name = tablename;

    if (check(tablename, tuple_in)) //判斷是否能夠插入
    {

        vector<Attribute> att = catalogmanager->getTableAttributes(tablename);
        vector<Index> index = catalogmanager->getIndexes(tablename);
        int length = catalogmanager->getTupleLength(name) + 1; //該表的tuple的長度和一位標誌位
                                                               //        cout << length << endl;
        //vector<Attribute> att(3);
        //att[0].attrName = "int"; att[1].attrName = "float"; att[2].attrName = "string";
        //att[0].attrType = -1;    att[1].attrType = 0;       att[2].attrType = 20;
        //att[0].isUnique = 0;     att[1].isUnique = 1;       att[2].isUnique = 0;
        //att[0].length = 16; att[1].length = 16; att[2].length = 20;
        //int length = 16 + 16 + 20 + 1;
        //int num = 3;

        Tuple tuple = tuple_in;
        vector<Data> record = tuple.getData();
        string insert = "";

        auto page_id = getpageid(file, length);              //尋找可以用的page_id
        auto page = buffermanager->fetchPage(page_id, file); //獲得page地址

        string temp;
        //把記錄定長儲存
        for (int i = 0; i < tuple.getSize(); i++)
        {
            insert.append(Datatostring(record[i], att[i])); //對數據整合為string
            if (att[i].isUnique)
            {
                switch (att[i].attrType)
                {
                case -1:
                    temp = toString(record[i].datai);
                    break;
                case 0:
                    temp = toString(record[i].dataf);
                    break;
                default:
                    temp = record[i].datas;
                }
                uniqure[tablename].insert(make_pair(att[i].attrName, temp));
            }
        }
        insert = "1" + insert;
        string info = page->getContent();
        info.append(insert);
        snprintf(page->getContent(), PAGESIZE, info.c_str()); //把數據存入該page_id的最後
        page->setDirty(1);
        //buffermanager->flushPage(page_id, file); //把數據寫入硬盤

#ifdef TIME
        auto endTime = system_clock::now();
        auto duration = duration_cast<microseconds>(endTime - startTime);
        cout << "(" << setiosflags(ios::fixed) << setprecision(2)
             << (double)duration.count() / 1000.0 << " ms)" << endl;
#endif
        //更新index
        Position pos;
        pos.pageId = page_id;
        pos.offset = 0;
        for (int i = 0; i < index.size(); i++)
        {
            int j;
            for (j = 0; j < tuple.getSize() && index[i].attrName != att[j].attrName; j++)
                ; //确定key
            vector<Tuple> temp = selectRecord(tablename, pos, PAGESIZE / length);
            pos.offset += (temp.size() - 1) * (length + 1); //計算tuple的偏移量
            indexmanager->insert(tablename, index[i].indexName, record[j], pos);
            pos.offset = 0;
        }
        return 1;
    }
    else
    {
        throw logic_error("Unique error.");
    }
}

void RecordManager::deleteRecord(const string &tablename) //刪除該表所有記錄
{
    //更新index
    string name = tablename;
    vector<Attribute> att = catalogmanager->getTableAttributes(tablename);
    vector<Index> index = catalogmanager->getIndexes(tablename);
    int num = att.size(); //該表的attributes的個數

    //vector<Attribute> att(3);
    //att[0].attrName = "int"; att[1].attrName = "float"; att[2].attrName = "string";
    //att[0].attrType = -1;    att[1].attrType = 0;       att[2].attrType = 20;
    //att[0].isUnique = 0;     att[1].isUnique = 1;       att[2].isUnique = 0;
    //att[0].length = 16; att[1].length = 16; att[2].length = 20;
    //int length = 16 + 16 + 20 + 1;
    //int num = 3;

    vector<Tuple> record = selectRecord(tablename);

    for (int i = 0; i < index.size(); i++)
    {
        for (int j = 0; j < num; j++)
        {
            if (index[i].attrName == att[j].attrName)
                for (int k = 0; k < record.size(); k++)
                {
                    vector<Data> data;
                    data = record[k].getData();
                    indexmanager->remove(tablename, index[i].indexName, data[j]);
                }
        }
    }

    // 清除索引
    uniqure.erase(tablename);

    dropTableFile(tablename);
    createTableFile(tablename);
}

void RecordManager::deleteRecord(const string &tablename, const vector<Compare> &compare) //按compaere的要求刪除記錄
{
    vector<Tuple> table;
    string name = tablename;
    string file = "./data/" + tablename + ".txt";

    vector<Attribute> att = catalogmanager->getTableAttributes(tablename);
    vector<Index> index = catalogmanager->getIndexes(tablename);
    int num = att.size(); //該表的attributes的個數
    int length = catalogmanager->getTupleLength(name) + 1;

    /*
    vector<Index> index(1);
    index[0].attrName = "float"; index[0].indexName = "index"; index[0].tableName = tablename;*/

    //vector<Attribute> att(3);
    //att[0].attrName = "int"; att[1].attrName = "float"; att[2].attrName = "string";
    //att[0].attrType = -1;    att[1].attrType = 0;       att[2].attrType = 20;
    //att[0].isUnique = 0;     att[1].isUnique = 1;       att[2].isUnique = 0;
    //att[0].length = 16; att[1].length = 16; att[2].length = 20;
    //int length = 16 + 16 + 20 + 1;
    //int num = 3;

    int page_id = 0;
    int max_page = getpageid(file, length); //table已經用了幾page

    while (page_id <= max_page)
    {
        Position pos(page_id, 0);
        table = selectRecord(tablename, pos, PAGESIZE / length); //把該page的tuples取出

        for (int i = 0; i < table.size(); i++) //選出和compare符合的tuples
        {
            bool match = true;
            vector<Data> data = table[i].getData();
            for (int j = 0; j < compare.size() && match; j++)
            {
                int count = 0;
                while (att[count].attrName != compare[j].attrName)
                    count++;
                if (!isMatch(data[count], compare[j]))
                    match = false;
            }
            if (match)
            {
                // 清除索引
                if (uniqure.find(tablename) != uniqure.end())
                {
                    string temp;
                    for (int i = 0; i < att.size(); i++)
                    {
                        if (att[i].isUnique)
                        {
                            switch (att[i].attrType)
                            {
                            case -1:
                                temp = toString(data[i].datai);
                                break;
                            case 0:
                                temp = toString(data[i].dataf);
                                break;
                            default:
                                temp = data[i].datas;
                            }
                        }
                        uniqure[tablename].erase(make_pair(att[i].attrName, temp));
                    }
                }
                table[i].deleteData();
            }
        }

        auto page = buffermanager->fetchPage(page_id, file);
        string insert = "";

        for (int i = 0; i < table.size(); i++) //重新寫入page
        {
            vector<Data> data(25);
            string temp = "";
            data = table[i].getData();
            for (int j = 0; j < num; j++)
            {
                temp.append(Datatostring(data[j], att[j]));
            }
            if (table[i].getSize())
                temp = "1" + temp;
            else
            {
                temp = "0" + temp;
                for (int j = 0; j < index.size(); j++) //移除index
                {
                    for (int k = 0; k < att.size(); k++)
                        if (index[j].attrName == att[k].attrName)
                        {
                            indexmanager->remove(tablename, index[j].indexName, data[k]);
                        }
                }
            }
            insert = insert + temp;
        }
        string info = "";
        info.append(insert);
        snprintf(page->getContent(), PAGESIZE, info.c_str());
        page->setDirty(1);
        //buffermanager->flushPage(page_id, file); //把數據寫入硬盤
        page_id++;
    }
}

void RecordManager::deleteRecord(const string &tablename, const Position &pos, const int &number) //按Position刪除記錄
{
    // 没看懂这函数在干嘛，直接把索引删干净。
    uniqure.erase(tablename);

    vector<Tuple> table;
    string file = "./data/" + tablename + ".txt";
    string name = tablename;

    vector<Attribute> att = catalogmanager->getTableAttributes(tablename);
    vector<Index> index = catalogmanager->getIndexes(tablename);
    int num = catalogmanager->getTableAttrNum(name);
    int length = catalogmanager->getTupleLength(name) + 1;

    //vector<Attribute> att(3);
    //att[0].attrName = "int"; att[1].attrName = "float"; att[2].attrName = "string";
    //att[0].attrType = -1;    att[1].attrType = 0;       att[2].attrType = 20;
    //att[0].isUnique = 0;     att[1].isUnique = 1;       att[2].isUnique = 0;
    //att[0].length = 16; att[1].length = 16; att[2].length = 20;
    //int length = 16 + 16 + 20 + 1;
    //int num = 3;

    int page_id = pos.pageId;

    table = selectRecord(tablename, pos, PAGESIZE / length); //選出該page中的所有tuples

    auto page = buffermanager->fetchPage(page_id, file); //獲得page的地址
    string insert = "";
    int count = 0;
    for (int i = 0; i < table.size(); i++) //重新寫入該page
    {
        vector<Data> data;
        data = table[i].getData();
        string temp = "";
        for (int j = 0; j < num; j++)
        {

            temp.append(Datatostring(data[j], att[j]));
        }
        if (count >= number && table[i].getSize())
            temp = "1" + temp;
        else
        {
            temp = "0" + temp;
            for (int j = 0; j < index.size(); j++) //移除index
            {
                for (int k = 0; k < att.size(); k++)
                    if (index[j].attrName == att[k].attrName)
                    {
                        indexmanager->remove(tablename, index[j].indexName, data[k]);
                    }
            }
        }
        insert = insert + temp;
        if (table[i].getSize())
            count++;
    }
    string info = "";
    info.append(insert);
    snprintf(page->getContent(), PAGESIZE, info.c_str());
    page->setDirty(1);
    //buffermanager->flushPage(page_id, file); //把數據寫入硬盤
    page_id++;
}

vector<Tuple> RecordManager::selectRecord(const string &tablename) //選取該表所有記錄
{
    string file = "./data/" + tablename + ".txt";
    string name = tablename;

    vector<Attribute> att = catalogmanager->getTableAttributes(tablename);
    int length = catalogmanager->getTupleLength(name) + 1;

    //vector<Attribute> att(3);
    //att[0].attrName = "int"; att[1].attrName = "float"; att[2].attrName = "string";
    //att[0].attrType = -1;    att[1].attrType = 0;       att[2].attrType = 20;
    //att[0].isUnique = 0;     att[1].isUnique = 1;       att[2].isUnique = 0;
    //att[0].length = 16; att[1].length = 16; att[2].length = 20;
    //int length = 16 + 16 + 20 + 1;
    //int num = 3;

    vector<string> temp;
    vector<Tuple> record;
    string info, str;

    auto page_id = getpageid(file, length);

    for (int i = 0; i <= page_id; i++) //把所有tuples從pages中讀出並轉換成Tuples
    {
        auto page = buffermanager->fetchPage(i, file);
        info = page->getContent();
        int j = 0;

        while (info[j] != 0 && j + length < PAGESIZE)
        {
            if (info[j] == '1')
            {
                str = info.substr(j + 1, length - 1);
                record.push_back(readTuple(str, att));
            }
            j = j + length;
        }
    }

    return record;
}

vector<Tuple> RecordManager::selectRecord(const string &tablename, const vector<Compare> &compare) //按compare來選擇記錄
{
#ifdef DEBUG
    for (auto &c : compare)
    {
        cout << c.attrName << ' ';
        switch (c.data.type)
        {
        case -1:
            cout << c.data.datai << endl;
        case 0:
            cout << c.data.dataf << endl;
        default:
            cout << c.data.datas << endl;
        }
    }
#endif
    vector<Tuple> table;
    table = selectRecord(tablename);

    vector<Attribute> att = catalogmanager->getTableAttributes(tablename);
    vector<Index> index = catalogmanager->getIndexes(tablename);
    //vector<Attribute> att(3);
    //att[0].attrName = "int"; att[1].attrName = "float"; att[2].attrName = "string";
    //att[0].attrType = -1;    att[1].attrType = 0;       att[2].attrType = 20;
    //att[0].isUnique = 0;     att[1].isUnique = 1;       att[2].isUnique = 0;
    //att[0].length = 16; att[1].length = 16; att[2].length = 20;
    //int length = 16 + 16 + 20 + 1;
    //int num = 3;
    vector<Tuple> record;
    vector<Position> pos;
    vector<Position> equal;
    Data left, right;
    left.type = right.type = 266;
    for (int i = 0; i < compare.size(); i++)
    {
        int j;
        for (j = 0; j < index.size() && compare[i].attrName != index[j].indexName; j++)
            ;
        if (j == index.size())
            break;
        switch (compare[i].type)
        {
        case (CompareType::E):
        case (CompareType::NE):
            pos = indexmanager->search(tablename, index[j].indexName, compare[i].data);
            break;
        case (CompareType::G):
            equal = indexmanager->search(tablename, index[j].indexName, compare[i].data);
        case (CompareType::GE):
            pos = indexmanager->search(tablename, index[j].indexName, compare[i].data, right);
            break;
        case (CompareType::L):
            equal = indexmanager->search(tablename, index[j].indexName, compare[i].data);
        case (CompareType::LE):
            pos = indexmanager->search(tablename, index[j].indexName, left, compare[i].data);
            break;
        }
        for (int k = 0; k < pos.size(); k++)
        {
            bool flag = true;
            if (compare[i].type != CompareType::NE || !equal.empty())
            {
                int z;
                for (z = 0; z < equal.size() && pos[k].offset != equal[z].offset && pos[k].pageId != equal[z].pageId; z++)
                    ;
                if (z != equal.size())
                    flag = false;
            }
            if (flag)
            {
                vector<Tuple> temp = selectRecord(tablename, pos[k]);
                record.push_back(temp[0]);
            }
        }
        if (i == compare.size() - 1)
            return record;
    }

    vector<Compare> com = compare;
    for (int i = 0; i < compare.size(); i++)
    {
        int j;
        for (j = 0; j < index.size() && compare[i].attrName != index[j].indexName; j++)
            ;
        if (j == index.size())
            break;
        com[i].attrName = index[j].attrName;
    }

    table = selectRecord(tablename);
    for (int i = 0; i < table.size(); i++) //選出符合compare的tuples
    {
        bool match = true;
        vector<Data> data = table[i].getData();
        for (int j = 0; j < com.size() && match; j++)
        {
            int count = 0;
            while (att[count].attrName != com[j].attrName)
                count++;
            if (!isMatch(data[count], com[j]))
                match = false; //檢查是否符合compare
        }
        if (!match)
            table[i].deleteData();
    }

    for (int i = 0; i < table.size(); i++)
    {
        if (table[i].getSize())
            record.push_back(table[i]);
    }

    return record;
}

vector<Tuple> RecordManager::selectRecord(const string &tablename, const Position &pos, const int number) //按Position來選擇記錄
{
    string file = "./data/" + tablename + ".txt";
    string name = tablename;
    vector<Tuple> record;

    vector<Attribute> att = catalogmanager->getTableAttributes(tablename);
    int length = catalogmanager->getTupleLength(name) + 1;

    //vector<Attribute> att(3);
    //att[0].attrName = "int"; att[1].attrName = "float"; att[2].attrName = "string";
    //att[0].attrType = -1;    att[1].attrType = 0;       att[2].attrType = 20;
    //att[0].isUnique = 0;     att[1].isUnique = 1;       att[2].isUnique = 0;
    //att[0].length = 16; att[1].length = 16; att[2].length = 20;
    //int length = 16 + 16 + 20 + 1;
    //int num = 3;
    auto page = buffermanager->fetchPage(pos.pageId, file);
    string info = page->getContent(), str;
    int j = pos.offset > 0 ? pos.offset : 0;
    int count = 0;

    while (info.size() > j && info[j + 1] != 0 && j + length < PAGESIZE && count < number) //取出該page數個tuples
    {
        str = info.substr(j + 1, length - 1);
        record.push_back(readTuple(str, att));
        count++;
        j = j + length;
    }

    j = 0;
    count = 0;
    while (info.size() > j && info[j + 1] != 0 && j + length < PAGESIZE && count < number)
    {
        if (info[j] == '0')
            record[count].deleteData();
        count++;
        j = j + length;
    }

    return record;
}

bool RecordManager::check(const string &tablename, const Tuple &tuple_in) //檢查tuple是否合法
{
#ifdef TIME
    auto startTime = system_clock::now();
#endif
    string name = tablename;

    vector<Attribute> att = catalogmanager->getTableAttributes(tablename);
    int num = att.size();

    //vector<Attribute> att(3);
    //att[0].attrName = "int"; att[1].attrName = "float"; att[2].attrName = "string";
    //att[0].attrType = -1;    att[1].attrType = 0;       att[2].attrType = 20;
    //att[0].isUnique = 0;     att[1].isUnique = 1;       att[2].isUnique = 0;
    //att[0].length = 16; att[1].length = 16; att[2].length = 20;
    //int length = 16 + 16 + 20 + 1;
    //int num = 3;

    Tuple t = tuple_in;
    vector<Data> data = t.getData();
    string temp;
    //    vector<Data> record = t.getData();
    //
    //    for (int i = 0; i < num; i++)   //attributes 是否一致
    //        switch (att[i].attrType) {
    //            case (-1):
    //            case (0):
    //                if (record[i].type != att[i].attrType) return false; break;
    //            default:
    //                if (record[i].type < 1 || record[i].type > 255) return false; break;
    //        }
    //
    for (int i = 0; i < num; i++) //有unique限制的是否有重複
    {
        if (att[i].isUnique)
        {
            // 没有，要新建索引
            if (uniqure.find(tablename) == uniqure.end())
            {
                auto tuples = selectRecord(tablename);
                for (auto &t : tuples)
                {
                    vector<Data> datas = t.getData();
                    for (int i = 0; i < datas.size(); i++)
                    {
                        if (att[i].isUnique)
                        {
                            switch (att[i].attrType)
                            {
                            case -1:
                                temp = toString(datas[i].datai);
                                break;
                            case 0:
                                temp = toString(datas[i].dataf);
                                break;
                            default:
                                temp = datas[i].datas;
                                break;
                            }
                            uniqure[tablename].insert(make_pair(att[i].attrName, temp));
                        }
                    }
                }
            }
            // 到这就有索引了
            switch (att[i].attrType)
            {
            case (-1):
                if (data[i].type != att[i].attrType)
                    return false;
                else
                    temp = toString(data[i].datai);
                break;
            case (0):
                if (data[i].type != att[i].attrType)
                    return false;
                else
                    temp = toString(data[i].dataf);
                break;
            default:
                if (data[i].type < 1 || data[i].type > 255)
                    return false;
                else
                    temp = data[i].datas;
            }
            if (uniqure[tablename].count(make_pair(att[i].attrName, temp)))
                return false;
            //            vector<Tuple> tableRecord = selectRecord(tablename);
            //            for (int j = 0; j < tableRecord.size(); j++)
            //            {
            //                vector<Data> temp = tableRecord[j].getData();
            //                if (isCompare(temp[i], record[i])) return false;
            //            }
        }
    }
#ifdef TIME
    auto endTime = system_clock::now();
    auto duration = duration_cast<microseconds>(endTime - startTime);
    cout << "check unique: (" << setiosflags(ios::fixed) << setprecision(2)
         << (double)duration.count() / 1000.0 << " ms)" << endl;
#endif
    return true;
}

int RecordManager::getpageid(const std::string &file, int length)
{
#ifdef TIME
    auto startTime = system_clock::now();
#endif
    int page_id = buffermanager->getFileLastPageId(file);
    // 空文件，newPage
    if (page_id == -1)
    {
        buffermanager->newPage(file, page_id);
        return page_id;
    }
    // 非空
    string info = buffermanager->fetchPage(page_id, file)->getContent();
    // 此页已满
    if (info.length() + length >= PAGESIZE)
    {
        buffermanager->newPage(file, page_id);
    }
#ifdef TIME
    auto endTime = system_clock::now();
    auto duration = duration_cast<microseconds>(endTime - startTime);
//    cout << "(" << setiosflags(ios::fixed) << setprecision(2)
//         << (double)duration.count() / 1000.0 << " ms)" << endl;
#endif
    return page_id;
}

void RecordManager::getIndexData(const string &tablename, int attrIndex, vector<Data> &datas, vector<Position> &positions)
{
    string file = "./data/" + tablename + ".txt";
    string name = tablename;

    vector<Attribute> att = catalogmanager->getTableAttributes(tablename);
    int length = catalogmanager->getTupleLength(name) + 1;

    string info, str;

    auto page_id = getpageid(file, length);

    for (int i = 0; i <= page_id; i++) //把所有tuples從pages中讀出並轉換成Tuples
    {
        auto page = buffermanager->fetchPage(i, file);
        info = page->getContent();
        int j = 0;

        while (info[j] != 0 && j + length < PAGESIZE)
        {
            if (info[j] == '1')
            {
                str = info.substr(j + 1, length - 1);
                Tuple tuple = readTuple(str, att);
                datas.emplace_back(tuple.getData(attrIndex));
                positions.emplace_back(Position(i, j));
            }
            j = j + length;
        }
    }
}
