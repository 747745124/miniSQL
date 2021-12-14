#ifndef MINISQL_BPTREENODE_H
#define MINISQL_BPTREENODE_H

#include "BpKey.h"

/*
 * Header: 32 Bytes
 *  4 * 7 = 28 Bytes
 *  由于内存对齐，isLeaf也占4 Bytes
 *
 * keys: 4K-32 Bytes at most
 */
struct BpTreeNodeHeader {
    PageIndex pageIndex;
    int keyCount, keySize;
    KeyType keyType;
    PageIndex parent, leftSibling, rightSibling;
    bool isLeaf;
};

class BpTreeNode {
private:
    // 一个Node里最多放三百多BpKey
    static const int MAXKEYS = 512;
public:
    BpTreeNodeHeader header{};
    // keys[0]在非叶节点只储存儿子指针，在叶节点无用
    BpKey *keys = nullptr;

    BpTreeNode();

    // 拷贝构造函数
    BpTreeNode(const BpTreeNode &node);

    ~BpTreeNode() { delete[] keys; }

    BpTreeNode &operator=(const BpTreeNode &node);

    int maxKeyCount() const;

    // 保存数据的大小（非占用内存大小）
    int getRawSize() const;

    void convertFromData(Byte *data);

    void writeToData(Byte *data) const;

    bool isOverflow() const;

    bool isUnderflow() const;

    bool hasSpare() const;

    bool insertKey(const BpKey &key);

    bool removeKey(const BpKey &key);

    // 找到key对应的Position（叶节点是record记录位置，非叶节点是node指针）
    Position getPosition(const BpKey &key) const;

    // 作为非叶节点时，找到key应在的儿子对应的position，
    /*
     * keys[0]在非叶节点只储存儿子指针，在叶节点无用
     * key0-pos0   key1-pos1 ...
     *  |    |      |    |
     * null son1   val1 son2
     */
    Position getSonPosition(const BpKey &key) const;
};


#endif //MINISQL_BPTREENODE_H
