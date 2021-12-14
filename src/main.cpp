#include <cassert>
#include <chrono>
#include <iomanip>
#include <iostream>

#include "API/API.h"
#include "CatalogManager/CatalogManager.h"
#include "IndexManager/BpTree.h"
#include "IndexManager/IndexManager.h"
#include "Interpreter/Interpreter.h"
#include "basic.h"

using namespace std;
using namespace chrono;

void testIndex();

void testCatalog();

void testAll();

void testTest();

void outputTuples();

int main() {
    ios::sync_with_stdio(false);
#ifdef DEBUG
    system("del /q .\\data\\*");
#endif
#ifdef TEST_INDEX
    testIndex();
    return 0;
#endif


    auto in = new Interpreter();
    string s, query;

    cout << "minisql> ";
    while (getline(cin, s)) {
        bool run = false;
        if (s.find(';') != string::npos) {
            s.erase(s.find_first_of(';') + 1);
            run = true;
        }
        query += s + " ";
        if (run) {
            auto startTime = system_clock::now();
            try {
                in->setQuery(query);
                if (in->runQuery() == 0)  // quit
                    break;

                auto endTime = system_clock::now();
                auto duration =
                    duration_cast<milliseconds>(endTime - startTime);
                cout << "(Total time: " << setiosflags(ios::fixed)
                     << setprecision(2) << (double)duration.count() / 1000.0
                     << " sec)" << endl
                     << endl;
            } catch (exception &e) {
                cout << e.what() << endl;
            }
            query.clear();
            cout << "minisql> ";
        } else {
            cout << "      -> ";
        }
    }

    delete in;

    //    testTest();
    //    testAll();
    //    testCatalog();
}

void testTest() {
    auto api = new API();

    vector<Attribute> attrs;
    attrs.emplace_back("height", 0, 16, true);
    attrs.emplace_back("pid", -1, 16, true);
    attrs.emplace_back("name", 32, 32, false);
    attrs.emplace_back("identity", 128, 128, true);
    attrs.emplace_back("age", -1, 16, true);

    api->createTable("person", attrs, "pid");

    Tuple tuple;
    tuple.addData((double)171.1);
    tuple.addData((int)1);
    tuple.addData(string("Person1"));
    tuple.addData(string("000001"));
    tuple.addData((int)81);

    api->insertRecord("person", tuple);
}

void testAll() {
    auto api = new API();

    vector<Attribute> attrs;
    attrs.emplace_back("name", 15, 15, false);
    attrs.emplace_back("id", -1, 16, true);

    api->createTable("person", attrs, "id");

    api->createIndex("person", "idx_name", "name");

    static Tuple tuples[10000];
    for (int i = 0; i < 10; i++) {
        if (i % 3 == 0)
            tuples[i].addData(string("Alice"));
        else if (i % 3 == 1)
            tuples[i].addData(string("Bob"));
        else
            tuples[i].addData(string("Catalina"));
        tuples[i].addData((int)i);
        try {
            api->insertRecord("person", tuples[i]);
        } catch (exception &e) {
            cerr << e.what() << endl;
        }
        api->getIndexManager()->printTree("person", "idx_name");
    }

    vector<Compare> idE5;
    idE5.emplace_back("id", CompareType::E, Data((int)5));
    api->deleteRecord("person", idE5);

    vector<Compare> compares;
    compares.emplace_back("id", CompareType::GE, Data((int)5));
    //    api->deleteRecord("person", compares);

    auto res = api->selectRecord("person", compares);

    cout << "Result:" << endl;
    for (auto &t : res) {
        auto ds = t.getData();
        for (auto &d : ds) {
            switch (d.type) {
                case -1:
                    cout << d.datai << ' ';
                    break;
                case 0:
                    cout << d.dataf << ' ';
                    break;
                default:
                    cout << d.datas << ' ';
                    break;
            }
        }
        cout << endl;
    }

    BufferPoolManager *b = api->getBufferManager();

    b->flushAllPages();  // 等析构再flush就来不及输出了
    // for (auto &i : b->count_read) {
    //     cout << "read: " << i.first << ": " << i.second << endl;
    // }
    // for (auto &i : b->count_flush) {
    //     cout << "write: " << i.first << ": " << i.second << endl;
    // }

    delete api;
}

void testCatalog() {
    auto catalog = new CatalogManager();
    Table table;
    table.tableName = "person";
    table.primaryKey = "id";
    table.attributes.emplace_back("id", -1, 16, true);
    table.attributes.emplace_back("name", 32, 32, false);
    assert(catalog->createTable(table));

    auto table2 = table;
    table2.tableName = "person2";
    assert(catalog->createTable(table2));

    Index index1("idx_id", "person2", "id");
    assert(catalog->createIndex(index1));

    assert(catalog->isIndexKey(table2.tableName, "id"));

    assert(!catalog->isIndexKey(table.tableName, "id"));
}

void testBuffer() {}

void testIndex() {
    BufferPoolManager *buffer = new BufferPoolManager(BUFFER_POOL_SIZE);
    string str =
        "123456789adjikhfgkhijsdasdrfgijkuy1adjikhfgkhijsdasdrfgijkuy123456789a"
        "djikhfgkhijsdasdrfgijkuy123456789adjikhfgkhijsdasdrfgijkuy";
    int strLen = 128;
    auto treeI = new BpTree(buffer, "table", "index", -1);
    const int N = 100;
    static BpKey keys[N];
    for (int i = 0; i < N; i++) {
        keys[i] = BpKey(Data((int)i), Position(i, i));
        next_permutation(str.begin(), str.end());
    }

    cerr << "begin--" << endl;
    int beginT = clock();
    for (int i = 0; i < N; i++) {
        treeI->insertKey(keys[i]);
        if (i % 1000 == 0) cerr << i << endl;
        treeI->printAllTree();
        cout << "-----------------------" << endl;
        if (!treeI->check()) {
            exit(-1);
        }
    }
    cout << "-----------------------" << endl;
    for (int i = 0; i < N; i++) {
        treeI->removeKey(keys[i]);
        treeI->printAllTree();
        cout << "-----------------------" << endl;
        if (!treeI->check()) {
            exit(-2);
        }
    }

    int endT = clock();
    cout << "time: " << endT - beginT << endl;
}

void outputTuples(const vector<Tuple> &tuples) {
    for (auto &t : tuples) {
        auto ds = t.getData();
        for (auto &d : ds) {
            switch (d.type) {
                case -1:
                    cout << d.datai << ' ';
                    break;
                case 0:
                    cout << d.dataf << ' ';
                    break;
                default:
                    cout << d.datas << ' ';
                    break;
            }
        }
        cout << endl;
    }
}