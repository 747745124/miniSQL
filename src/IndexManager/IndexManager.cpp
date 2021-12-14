#include "IndexManager.h"
#include "../RecordManager/RecordManager.h"
#include <cassert>

IndexManager::IndexManager() = default;

IndexManager::~IndexManager() = default;

bool IndexManager::createIndex(const string &tableName, const string &indexName,
                               const string &attrName) {


    vector<Data> datas;
    vector<Position> positions;
    int attrType = -2;
    int attrIndex = -1;

    vector<Attribute> attrs = catalog->getTableAttributes(tableName);
    for (auto a = attrs.begin(); a != attrs.end(); a++) {
        if (a->attrName == attrName) {
            attrType = a->attrType;
            attrIndex = a - attrs.begin();
            break;
        }
    }
#ifdef DEBUG
    cout << "IM: Create" << endl;
    cout << "IM: attr type: " << attrType << endl;
    cout << "IM: attr index: " << attrIndex << endl;
#endif

    // API会检查
//    if (catalog->isIndexExist(indexName)) {
//        return false;
//    } else
    {
        auto newTree = new BpTree(buffer, tableName, indexName, attrType);
        treeMap[make_pair(tableName, indexName)] = newTree;
        // TODO: 从record获取此属性的所有值和Position，插入索引
        record->getIndexData(tableName, attrIndex, datas, positions);

        return createIndex(tableName, indexName, datas, positions);
    }
}

bool IndexManager::dropIndex(const string &tableName, const string &indexName) {
    if (catalog->isIndexExist(indexName) == false) return false;

    // TODO: buffer删除Index文件
    buffer->deleteFileAndPages("./data/" + tableName + "_" + indexName + ".idx");
    // TODO: catalog删除Index记录（不在这里实现）
    auto i = treeMap.find(make_pair(tableName, indexName));
    if (i != treeMap.end()) {
        delete i->second;
        treeMap.erase(i);
    }
    return true;
}

bool IndexManager::insert(const string &tableName, const string &indexName,
                          const Data &data, Position position) {
    if (!catalog->isIndexExist(indexName)) return false;

    BpTree *tree = treeMap[make_pair(tableName, indexName)];
    if (tree == nullptr)
        tree = treeMap[make_pair(tableName, indexName)] = new BpTree(buffer, tableName, indexName, data.type);

    return tree->insertKey(BpKey(data, position));
}

bool IndexManager::remove(const string &tableName, const string &indexName,
                          const Data &data) {
    if (catalog->isIndexExist(indexName)) return false;

    BpTree *tree = treeMap[make_pair(tableName, indexName)];
    if (tree == nullptr)
        tree = treeMap[make_pair(tableName, indexName)] = new BpTree(buffer, tableName, indexName, data.type);

    return tree->removeKey(BpKey(data, Position()));
}

std::vector<Position>
IndexManager::search(const string &tableName, const string &indexName,
                     const Data &left, const Data &right) {

    // 检查是否存在此index，如果不存在返回空
    if (catalog->isIndexExist(indexName) == false) return vector<Position>();

    BpTree *tree = treeMap[make_pair(tableName, indexName)];
    // 这里type没有写，会从文件读入
    if (tree == nullptr)
        tree = treeMap[make_pair(tableName, indexName)] = new BpTree(buffer, tableName, indexName);

    vector<Position> positions;
    tree->search(BpKey(left), BpKey(right), positions);
    return positions;
}

std::vector<Position>
IndexManager::search(const string &tableName, const string &indexName,
                     const Data &equal) {
    // 检查是否存在此index，如果不存在返回空
    if (catalog->isIndexExist(indexName)) return vector<Position>();

    BpTree *tree = treeMap[make_pair(tableName, indexName)];
    // 这里type没有写，会从文件读入
    if (tree == nullptr)
        tree = treeMap[make_pair(tableName, indexName)] = new BpTree(buffer, tableName, indexName);

    vector<Position> positions;
    tree->search(BpKey(equal), positions);
    return positions;
}

bool IndexManager::createIndex(const string &tableName, const string &indexName,
                               const vector<Data> &datas, const vector<Position> &positions) {

    BpTree *tree = treeMap[make_pair(tableName, indexName)];
    assert(tree != nullptr); // 理论上这个函数只会被另一个createIndex调用，调用之前会整一个treeMap[make_pair(tableName, indexName)]
    assert(datas.size() == positions.size());
    for (int i = 0; i < datas.size(); i++)
        assert(insert(tableName, indexName, datas[i], positions[i]));
#ifdef DEBUG
    cout << "Number of tuples: " << datas.size() << endl;
    printTree(tableName, indexName);
#endif
    return true;
}
