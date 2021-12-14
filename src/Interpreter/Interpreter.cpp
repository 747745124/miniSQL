#include "Interpreter.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <iomanip>
#include <iostream>

using namespace std;
using namespace chrono;

const string Interpreter::QUIT = "quit";
const string Interpreter::CREATE = "create";
const string Interpreter::DROP = "drop";
const string Interpreter::INSERT = "insert";
const string Interpreter::DELETE = "delete";
const string Interpreter::SELECT = "select";
const string Interpreter::EXECFILE = "execfile";

const string Interpreter::INDEX = "index";
const string Interpreter::TABLE = "table";
const string Interpreter::VALUES = "values";

const string Interpreter::INTO = "into";
const string Interpreter::ON = "on";
const string Interpreter::FROM = "from";
const string Interpreter::UNIQUE = "unique";
const string Interpreter::PRIMARY = "primary";
const string Interpreter::KEY = "key";

const string Interpreter::FLOAT = "float";
const string Interpreter::INT = "int";
const string Interpreter::CHAR = "char";

Interpreter::Interpreter() { api = new API(); }

Interpreter::~Interpreter() { delete api; }

// 拆分单词、关键词转小写
void Interpreter::setQuery(string q)
{
    query = q;
    replace(query.begin(), query.end(), '\"', '\'');
    words.clear();
    isString.clear();

    // 符号前后添加空格，方便拆分
    for (int i = 0; i < query.length(); i++)
    {
        // Add spaces before and after != <> <= >= = < > ( ) * , ;
        if (query[i] == '=' || query[i] == '<' || query[i] == '>' ||
            query[i] == '(' || query[i] == ')' || query[i] == '*' ||
            query[i] == ',' || query[i] == ';' || query[i] == '!')
        {
            if (query[i - 1] != ' ')
            {
                query.insert(i, " ");
                i++;
            }

            if (query[i + 1] == '=' || query[i + 1] == '>')
            { // != <> <= >=
                if (query[i + 2] != ' ')
                {
                    query.insert(i + 2, " ");
                }
                i++;
            }
            else
            {
                if (query[i + 1] != ' ')
                {
                    query.insert(i + 1, " ");
                }
            }
        }
    }

    stringstream qStream(query);
    string word;
    while (qStream >> word)
    {
        if (word[0] == '(' || word[0] == ')' || word[0] == ',' ||
            word[0] == ';')
            continue;
        // 关键词全部转小写
        if (isKeyWord(word))
            toLower(word);
        if (word[0] == '\'')
        {
            while (word[word.length() - 1] != '\'')
            {
                string temp;
                if (!(qStream >> temp))
                    throw runtime_error("Error: Syntax error on string.");
                word += " " + temp;
            }
            word = word.substr(1, word.length() - 2);
            isString.push_back(1);
        }
        else
            isString.push_back(0);
        words.push_back(word);
    }

#ifdef DEBUG
    cerr << "Debug message:" << endl;
    cerr << "setQuery:" << endl;
    for (auto &i : words)
        cerr << "`" << i << "`" << ' ';
    cerr << endl;
    cerr << "--------------" << endl;
#endif
}

int Interpreter::runQuery()
{
    if (!words.size())
        return -1;

    string operation = words.at(0);
    if (operation == QUIT)
        return 0;
    else if (operation == EXECFILE)
    {
        runFile();
    }
    else if (operation == CREATE)
    {
        string obj = words.at(1);
        if (obj == TABLE)
            createTable();
        else if (obj == INDEX)
            createIndex();
        else
            throw logic_error("Error: CREATE not TABLE or INDEX.");
    }
    else if (operation == DROP)
    {
        string obj = words.at(1);
        if (obj == TABLE)
            dropTable();
        else if (obj == INDEX)
            dropIndex();
        else
            throw logic_error("Error: DROP not TABLE or INDEX.");
    }
    else if (operation == INSERT)
    {
        if (words.at(1) != INTO || words.at(3) != VALUES)
            throw logic_error("Error: Syntax error on INSERT.");
        insert();
    }
    else if (operation == DELETE)
    {
        if (words.at(1) != FROM)
            throw logic_error("Error: Syntax error on DELETE.");
        string tableName = words.at(2);
        vector<Compare> compares;
        getCompares(compares, 4);
        // DO: API.delete
        api->deleteRecord(tableName, compares);
    }
    else if (operation == SELECT)
    {
        if (words.at(1) != "*")
            throw logic_error(
                "Shit: This garbage SQL does not support such complex "
                "command!!!");
        if (words.at(2) != FROM)
            throw logic_error("Error: Syntax error on SELECT.");
        string tableName = words.at(3);
        vector<Compare> compares;
        getCompares(compares, 5);
#ifdef DEBUG
        for (auto &c : compares)
        {
            cout << c.attrName << ' ';
            switch (c.data.type)
            {
            case -1:
                cout << c.data.datai << endl;
                break;
            case 0:
                cout << c.data.dataf << endl;
                break;
            default:
                cout << c.data.datas << endl;
                break;
            }
        }
#endif
        // DO: API.select
        // DO: catalog.getTableAttributes
        vector<Tuple> tuples;
        vector<Attribute> attrs;
        attrs = api->getCatalogManager()->getTableAttributes(tableName);
        tuples = api->selectRecord(tableName, compares);
#ifdef NOAPI
        attrs.push_back(Attribute("identity", -1, 16, 1));
        attrs.push_back(Attribute("name", 32, 32, 0));
        Tuple tuple1;
        tuple1.addData(Data((int)123));
        tuple1.addData(Data(string("Alice")));
        Tuple tuple2;
        tuple2.addData(Data((int)456));
        tuple2.addData(Data(string("Bob")));
        tuples.push_back(tuple1);
        tuples.push_back(tuple2);
#endif
        showTable(tuples, attrs);
    }
    else
    {
        throw logic_error("Error: Invalid operation.");
    }

    return 1;
}

void Interpreter::createTable()
{
    string tableName = words[2];
    vector<Attribute> attrs;
    string primaryKey;
    bool hasPrimaryKey = false;

    int i = 3;
    while (i < words.size())
    {
        // primary key
        if (i + 1 < words.size() && words.at(i) == PRIMARY &&
            words.at(i + 1) == KEY)
        {
            if (hasPrimaryKey)
                throw logic_error("Error: Multiple primary keys.");
            // 设置主键
            i += 2;
            hasPrimaryKey = true;
            primaryKey = words.at(i++);
        }
        else
        { // attr
            Attribute attr;
            attr.attrName = words.at(i);
            if (words.at(i + 1) == INT)
            {
                attr.attrType = -1;
                attr.length = 16;
                i += 2;
            }
            else if (words.at(i + 1) == FLOAT)
            {
                attr.attrType = 0;
                attr.length = 16;
                i += 2;
            }
            else if (words.at(i + 1) == CHAR)
            {
                int strLen = atoi(words.at(i + 2).c_str());
                if (strLen < 1 || strLen > 255)
                {
                    throw logic_error("Error: Invalid string length.");
                }
                attr.attrType = strLen;
                attr.length = strLen;
                i += 3;
            }
            else
            {
                throw logic_error("Error: Invalid type.");
            }

            // unique
            if (i < words.size() && words.at(i) == UNIQUE)
            {
                attr.isUnique = true;
                i++;
            }
            else if (i + 1 < words.size() && words.at(i) == PRIMARY &&
                     words.at(i + 1) == KEY && i + 2 != words.size() - 1)
            {
                // 最后一个条件是 PRIMARY KEY (xxx)的时候这个primary
                // key不能被认为是上一个的
                if (hasPrimaryKey)
                    throw logic_error("Error: Multiple primary keys.");
                hasPrimaryKey = true;
                primaryKey = attr.attrName;
                attr.isUnique = true;
                i += 2;
            }
            else
            {
                attr.isUnique = false;
            }

            //            cerr << attr.attrName << ' ' << attr.attrType << ' '
            //            << attr.isUnique << ' '
            //                 << attr.length << endl;
            attrs.push_back(attr);
        }
    }

    if (hasPrimaryKey)
    {
        for (auto &a : attrs)
        {
            if (a.attrName == primaryKey)
            {
                a.isUnique = true;
                break;
            }
        }
    }

#ifdef DEBUG
    cerr << "Debug message:" << endl;
    cerr << "CREATE TABLE" << endl;
    cerr << "tableName: " << tableName << endl;
    cerr << "attributes: name,type,unique,length" << endl;
    for (auto &i : attrs)
    {
        cerr << i.attrName << ' ' << i.attrType << ' ' << i.isUnique << ' '
             << i.length << endl;
    }
    cerr << "-------------" << endl;
#endif

    // DO: API.createTable
    // tableName attrs
    if (hasPrimaryKey)
        api->createTable(tableName, attrs, primaryKey);
    else
        api->createTable(tableName, attrs);
}

void Interpreter::createIndex()
{
    if (words[3] != ON)
        throw logic_error("Error: Sytanx error.");

    string indexName = words.at(2);
    string tableName = words.at(4);
    string attrName = words.at(5);

#ifdef DEBUG
    cerr << "Debug message:" << endl;
    cerr << "CREATE INDEX" << endl;
    cerr << "index name: " << indexName << endl;
    cerr << "table name: " << tableName << endl;
    cerr << "attri name: " << attrName << endl;
    cerr << "--------------" << endl;
#endif

    // DO: API.createIndex
    api->createIndex(tableName, indexName, attrName);
}

void Interpreter::dropTable()
{
    string tableName = words.at(2);
    // DO: API.dropTable
    api->dropTable(tableName);

#ifdef DEBUG
    cerr << "Debug message:" << endl;
    cerr << "DROP TABLE" << endl;
    cerr << "table name: " << tableName << endl;
    cerr << "--------------" << endl;
#endif
}

void Interpreter::dropIndex()
{
    string indexName = words.at(2);
    // DO: API.dropIndex
    api->dropIndex(indexName);
#ifdef DEBUG
    cerr << "Debug message:" << endl;
    cerr << "DROP INDEX" << endl;
    cerr << "index name: " << indexName << endl;
    cerr << "--------------" << endl;
#endif
}

void Interpreter::insert()
{
    string tableName = words.at(2);
    vector<Data> datas;
    vector<Attribute> attrs;
    // DO: attrs = catalog->getTableAttributes(tableName);
    attrs = api->getCatalogManager()->getTableAttributes(tableName);
#ifdef NOAPI
    attrs.push_back(Attribute("name", 32, 32, 0));
    attrs.push_back(Attribute("height", 0, 16, 0));
#endif
    if (words.size() - 4 != attrs.size())
        throw logic_error("Error: Number of attributes doesn't match.");

    for (int i = 4; i < words.size(); i++)
    {
        if (isString.at(i))
        {
            if (attrs.at(i - 4).attrType < 1 || attrs.at(i - 4).attrType > 255)
                throw logic_error("Error: Attribute type doesn't match.");
            Data data(words.at(i));
            data.type = attrs.at(i - 4).attrType;
            datas.push_back(data);
        }
        else
        {
            // 有小数点但不是float
            if (words.at(i).find('.') != string::npos &&
                attrs.at(i - 4).attrType != 0)
            {
                throw logic_error("Error: Attribute type doesn't match.");
            }
            Data data;
            if (attrs.at(i - 4).attrType == -1)
            {
                data.datai = atoi(words.at(i).c_str());
                data.type = -1;
            }
            else if (attrs.at(i - 4).attrType == 0)
            {
                data.dataf = atof(words.at(i).c_str());
                data.type = 0;
            }
            datas.push_back(data);
        }
    }
    Tuple tuple;
    for (auto &d : datas)
        tuple.addData(d);

    // DO: API.insert
    api->insertRecord(tableName, tuple);
#ifdef DEBUG
    cerr << "Debug message:" << endl;
    cerr << "INSERT" << endl;
    cerr << "table name: " << tableName << endl;
    for (auto &i : datas)
    {
        if (i.type == -1)
            cerr << i.datai << ' ';
        else if (i.type == 0)
            cerr << i.dataf << ' ';
        else
            cerr << i.datas << ' ';
    }
    cout << endl;
    cerr << "--------------" << endl;
#endif
}

void Interpreter::getCompares(vector<Compare> &compares, int i)
{
    while (i < words.size())
    {
        Compare compare;

        if (words.at(i) == "and")
            i++;
        compare.attrName = words.at(i++);

        string operation = words.at(i++);
        if (operation == "=")
            compare.type = CompareType::E;
        else if (operation == "<")
            compare.type = CompareType::L;
        else if (operation == ">")
            compare.type = CompareType::G;
        else if (operation == "<=")
            compare.type = CompareType::LE;
        else if (operation == ">=")
            compare.type = CompareType::GE;
        else if (operation == "!=" || operation == "<>")
            compare.type = CompareType::NE;
        else
        {
            throw logic_error("Error: Syntax error on WHERE.");
        }

        // !!! 这里不一定和attr一样，string长度可能不一样，double可能用int表示
        if (isString[i])
        {
            compare.data = Data(words.at(i));
        }
        else if (words.at(i).find('.') != string::npos)
        {
            compare.data = Data(atof(words.at(i).c_str()));
        }
        else
        {
            compare.data = Data(atoi(words.at(i).c_str()));
        }
        compares.push_back(compare);
        i++;
    }
#ifdef DEBUG
    cerr << "Debug message:" << endl;
    cerr << "Compares:" << endl;
    for (auto &i : compares)
    {
        cerr << i.attrName << " ";
        switch (i.type)
        {
        case CompareType::E:
            cerr << "E ";
            break;
        case CompareType::GE:
            cerr << "GE ";
            break;
        case CompareType::LE:
            cerr << "LE ";
            break;
        case CompareType::NE:
            cerr << "NE ";
            break;
        case CompareType::G:
            cerr << "G ";
            break;
        case CompareType::L:
            cerr << "L ";
            break;
        }
        switch (i.data.type)
        {
        case -1:
            cerr << i.data.datai << endl;
            break;
        case 0:
            cerr << i.data.dataf << endl;
            break;
        default:
            cerr << i.data.datas << endl;
            break;
        }
    }
    cerr << "--------------" << endl;
#endif
}

void Interpreter::showTable(const vector<Tuple> &tuples,
                            const vector<Attribute> &attrs)
{
    //    assert(tuples.size() == attrs.size());

    int cnt = attrs.size();
    vector<int> length(cnt, 0);
    vector<int> isNum(cnt, 0);
    vector<string> temp(cnt);
    vector<vector<string>> table;

    // 表头
    for (int i = 0; i < cnt; i++)
    {
        temp[i] = attrs[i].attrName;
        length[i] = max(length[i], (int)temp[i].length());
        isNum[i] = (attrs[i].attrType <= 0);
    }
    table.push_back(temp);

    for (auto &tuple : tuples)
    {
        assert(cnt == tuple.getSize());
        vector<Data> datas = tuple.getData();
        for (int i = 0; i < cnt; i++)
        {
            if (datas[i].type == -1)
            {
                stringstream ss;
                ss << datas[i].datai;
                ss >> temp[i];
            }
            else if (datas[i].type == 0)
            {
                stringstream ss;
                ss << datas[i].dataf;
                ss >> temp[i];
            }
            else
            {
                temp[i] = datas[i].datas;
            }
            length[i] = max(length[i], (int)temp[i].length());
        }
        table.push_back(temp);
    }

    string splitLine = "+";
    for (auto &l : length)
    {
        splitLine += string(l + 2, '-') + "+";
    }
    // 表头

    if (table.size() - 1 > 0)
    {
        cout << splitLine << endl;
        cout << "|";
        for (int j = 0; j < cnt; j++)
        {
            cout << format(table[0][j], length[j], 0) << "|";
        }
        cout << endl
             << splitLine << endl;
        for (int i = 1; i < table.size(); i++)
        {
            cout << "|";
            for (int j = 0; j < cnt; j++)
            {
                cout << format(table[i][j], length[j], isNum[j]) << "|";
            }
            cout << endl;
        }
        cout << splitLine << endl;
        cout << table.size() - 1 << " row"
             << (table.size() - 1 == 1 ? " " : "s ") << "in set." << endl;
    }
    else
    {
        cout << "Empty set." << endl;
    }
}

string Interpreter::format(const string &str, int length, int isNum)
{
    string res = str;
    if (res.length() < length)
    {
        if (isNum)
            res = string(length - res.length(), ' ') + res;
        else
            res = res + string(length - res.length(), ' ');
    }
    return " " + res + " ";
}

void Interpreter::runFile()
{
    string fileName = words.at(1);
    ifstream fin(fileName);
    if (!fin)
        throw runtime_error("Error: Failed to open file: " + fileName + ".");

    string temp, q;
    while (fin.peek() != EOF)
    {
        if (!getline(fin, temp))
            break;
        bool run = false;
        if (temp.find(';') != string::npos)
        {
            temp.erase(temp.find_first_of(';') + 1);
            run = true;
        }
        q += temp + " ";

        if (run)
        {
            auto startTime = system_clock::now();
            try
            {
                setQuery(q);
                runQuery();

                auto endTime = system_clock::now();
                auto duration =
                    duration_cast<microseconds>(endTime - startTime);
                cout << "(" << setiosflags(ios::fixed) << setprecision(2)
                     << (double)duration.count() / 1000.0 << " ms)" << endl;
            }
            catch (exception &e)
            {
                cout << e.what() << endl;
            }
            q.clear();
        }
    }
}
