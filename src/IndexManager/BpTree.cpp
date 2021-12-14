#include "BpTree.h"
#include "../BufferManager/page.h"

#include <stdexcept>
#include <cassert>
#include <iostream>


BpTree::BpTree(BufferPoolManager *bufferPoolManager, const string &tableName,
               const string &indexName, const int &indexType) {
    if (indexType < -1 || indexType > 255) {
        throw invalid_argument("Invalid indexType.");
    }

    buffer = bufferPoolManager;
    this->tableName = tableName;
    this->indexName = indexName;
    this->fileName = "./data/" + tableName + "_" + indexName + ".idx";
    header.nodeCount = 0;

    switch (indexType) {
        case -1:
            header.keyType = KeyType::INT;
            break;
        case 0:
            header.keyType = KeyType::FLOAT;
            break;
        default:
            header.keyType = KeyType::STRING;
    }
#ifdef DEBUG
    cout << "BpTree: create tree: type = " << (int)header.keyType << endl;
#endif
    header.keySize = BpKey::getRawSize(header.keyType, indexType);
    header.rootPage = 1;

    // 新索引，新建文件，写入header和根节点
    // 旧索引，从文件中读取header
    if (!buffer->pageExist(fileName, 0)) { // 无根，空文件
//        cout << "!!!";
        int headerIndex;
        Page *page = buffer->newPage(fileName, headerIndex);
        assert(headerIndex == 0); // 应该是0
        // 建根
        BpTreeNode *rootNode = createNode(true);
        header.rootPage = rootNode->header.pageIndex;
        header.nodeCount = 1;
        writeHeaderToData((Byte *) page->getContent());
//        buffer->flushPage(0, fileName);
        page->setDirty(true);
        delete rootNode;
    } else {
        Page *page = buffer->fetchPage(0, fileName);
        cout << page->getPageId() << endl;
        convertHeaderFromData((Byte *) page->getContent());
    }
}

void BpTree::convertHeaderFromData(Byte *data) {
    memcpy(&header, data, sizeof(header));
}

void BpTree::writeHeaderToData(Byte *data) const {
    memcpy(data, &header, sizeof(header));
}

BpTreeNode *BpTree::createNode(bool isLeaf) {
    int pageIndex;
    Page *page = buffer->newPage(fileName, pageIndex);

    // 新建节点并写入
    header.nodeCount++;
    auto *node = new BpTreeNode();
    node->header.pageIndex = pageIndex;
    node->header.keyCount = 0;
    node->header.keySize = header.keySize;
    node->header.keyType = header.keyType;
    node->header.parent = 0;
    node->header.leftSibling = 0;
    node->header.rightSibling = 0;
    node->header.isLeaf = isLeaf;

    node->writeToData((Byte *) page->getContent());
//    buffer->flushPage(page->getPageId(), fileName);
    page->setDirty(true);
    return node;
}

void BpTree::deleteNode(BpTreeNode *node) {
//    buffer->deletePage(fileName, node->header.pageIndex);
    header.nodeCount--;
    delete node;
}

BpTreeNode *BpTree::getNode(PageIndex pageIndex) {
    Page *page = buffer->fetchPage(pageIndex, fileName);
    auto node = new BpTreeNode();
    node->convertFromData((Byte *) page->getContent());
    return node;
}

BpTreeNode *BpTree::getNode(unordered_map<PageIndex, BpTreeNode *> &nodeMap, PageIndex pageIndex) {
    if (nodeMap.find(pageIndex) != nodeMap.end()) {
        return nodeMap[pageIndex];
    } else {
        auto node = getNode(pageIndex);
        nodeMap[pageIndex] = node;
        return node;
    }
}

bool BpTree::insertKey(const BpKey &key) {
    unordered_map<PageIndex, BpTreeNode *> nodeMap;

    auto *node = getNode(nodeMap, header.rootPage);

    // Find insert position
    while (!node->header.isLeaf) {
        Position pos = node->getSonPosition(key);
        node = getNode(nodeMap, pos.pageId);
    }
    node->insertKey(key);
    // 如果插入了第一个，要向上更新索引
    if (node->keys[1] == key) {
        updateNode(nodeMap, node->header.parent, node->keys[2], key);
    }

    // If overflow, split
    while (node->isOverflow()) {
        BpTreeNode *newNode = createNode(node->header.isLeaf);
        nodeMap[newNode->header.pageIndex] = newNode;
        splitNode(nodeMap, node, newNode);
        // split root
        if (node->header.parent == 0) {
            BpTreeNode *newRoot = createNode(false);
            header.rootPage = newRoot->header.pageIndex;
            nodeMap[newRoot->header.pageIndex] = newRoot;
            node->header.parent = newNode->header.parent = newRoot->header.pageIndex;
            if (node->header.isLeaf) {
                newRoot->keys[0] = node->keys[1];
                newRoot->keys[0].position = Position(node->header.pageIndex);
            } else {
                newRoot->keys[0] = node->keys[0];
                newRoot->keys[0].position = Position(node->header.pageIndex);
            }
        }
        // insert new key into parent
        BpTreeNode *parent = getNode(nodeMap, node->header.parent);
        BpKey insKey = node->header.isLeaf ? newNode->keys[1] : newNode->keys[0];
        insKey.position = Position(newNode->header.pageIndex, 0);
        parent->insertKey(insKey);

        node = getNode(nodeMap, node->header.parent);
    }

    // 插入结束，所有修改写回文件，删除BpTreeNode
    for (auto &i : nodeMap) {
        node = i.second;
        Page *page = buffer->fetchPage(node->header.pageIndex, fileName);
        assert(node->header.pageIndex == page->getPageId());
        node->writeToData((Byte *) page->getContent());
//        buffer->flushPage(page->getPageId(), fileName);
        page->setDirty(true);
        delete node;
    }
    Page *page = buffer->fetchPage(0, fileName);
    writeHeaderToData((Byte *) page->getContent());

    return true;
}

void BpTree::splitNode(unordered_map<PageIndex, BpTreeNode *> &nodeMap,
                       BpTreeNode *x, BpTreeNode *y) {
    int keyCount = x->header.keyCount;
    int midPos = keyCount / 2;

    if (x->header.rightSibling != 0) {
        BpTreeNode *rrNode = getNode(nodeMap, x->header.rightSibling);
        rrNode->header.leftSibling = y->header.pageIndex;
    }

    y->header.isLeaf = x->header.isLeaf;
    y->header.parent = x->header.parent;
    y->header.rightSibling = x->header.rightSibling;
    y->header.leftSibling = x->header.pageIndex;
    x->header.rightSibling = y->header.pageIndex;

    if (x->header.isLeaf) {
        x->header.keyCount = midPos;
        y->header.keyCount = keyCount - midPos;
        for (int i = midPos + 1; i <= keyCount; i++) {
            y->keys[i - midPos] = x->keys[i];
        }
    } else {
        x->header.keyCount = midPos;
        y->header.keyCount = keyCount - midPos - 1;
        for (int i = midPos + 1; i <= keyCount; i++) {
            y->keys[i - midPos - 1] = x->keys[i];
        }
        // update parent pointer
        for (int i = 0; i <= y->header.keyCount; i++) {
            BpTreeNode *son = getNode(nodeMap, y->keys[i].position.pageId);
            son->header.parent = y->header.pageIndex;
        }
    }
}

bool BpTree::removeKey(const BpKey &key) {
    unordered_map<PageIndex, BpTreeNode *> nodeMap;

    auto *node = getNode(nodeMap, header.rootPage);

    while (!node->header.isLeaf) {
        Position pos = node->getSonPosition(key);
        node = getNode(nodeMap, pos.pageId);
    }

    // No key. false
    if (!node->removeKey(key)) {
        for (auto &i : nodeMap) delete i.second;
        return false;
    }

    // if deleted the first, update index
    if (key < node->keys[1]) {
        updateNode(nodeMap, node->header.parent, key, node->keys[1]);
    }

    // underflow
    while (node->isUnderflow()) {
        // 根可以underflow
        if (node->header.pageIndex == header.rootPage)
            break;

        // 必须是同一个爹的亲兄弟
        BpTreeNode *lNode = getLNode(nodeMap, node),
                *rNode = getRNode(nodeMap, node);
        // 理论上至少有一个吧
        assert(lNode != nullptr || rNode != nullptr);

        bool fixed = false;
        // 借一个之后不用再向上
        if (lNode != nullptr && lNode->hasSpare()) {
            if (node->header.isLeaf)
                leafBorrowLeft(nodeMap, node, lNode);
            else nonleafBorrowLeft(nodeMap, node, lNode);
            fixed = true;
        } else if (rNode != nullptr && rNode->hasSpare()) {
            if (node->header.isLeaf)
                leafBorrowRight(nodeMap, node, rNode);
            else nonleafBorrowRight(nodeMap, node, rNode);
            fixed = true;
        }

        if (fixed) break;

        // merge
        if (lNode != nullptr) {
            if (node->header.isLeaf)
                leafMerge(nodeMap, lNode, node);
            else nonleafMerge(nodeMap, lNode, node);
            node = lNode;
        } else if (rNode != nullptr) {
            if (node->header.isLeaf)
                leafMerge(nodeMap, node, rNode);
            else nonleafMerge(nodeMap, node, rNode);
        }

        // root这一层不会再进行下去，可以直接break
        // 但是如果merge之后root里的key空了，就去掉root
        if (node->header.parent == header.rootPage) {
            auto root = getNode(nodeMap, node->header.parent);
            if (root->header.keyCount == 0) { // 去旧root
                // change root
                header.rootPage = node->header.pageIndex;
                // delete parent pointer
                node->header.parent = 0;
                // delete root
                nodeMap[root->header.pageIndex] = nullptr;
                deleteNode(root);
            }
            break;
        }

        node = getNode(nodeMap, node->header.parent);
    }

    // 删除结束，写回硬盘
    for (auto &i : nodeMap) {
        if (i.second != nullptr) {
            node = i.second;
            Page *page = buffer->fetchPage(node->header.pageIndex, fileName);
            node->writeToData((Byte *) page->getContent());
            assert(node->header.pageIndex == page->getPageId());
//            buffer->flushPage(page->getPageId(), fileName);
            page->setDirty(true);
            delete node;
        }
    }
    Page *page = buffer->fetchPage(0, fileName);
    writeHeaderToData((Byte *) page->getContent());

    return true;
}

// !!! 对于 m<=4 即允许节点只储存1个值的情况可能出错
// 这里m最小15，所以没有考虑
void BpTree::updateNode(unordered_map<PageIndex, BpTreeNode *> &nodeMap,
                        PageIndex pageIndex, const BpKey &key1, const BpKey &key2) {
    if (pageIndex == 0) return;
    if (key1 == key2) return;
    BpTreeNode *node = getNode(nodeMap, pageIndex);

    // 子树最小还要继续向上更新
    if (key1 < node->keys[1]) {
        assert(key1 == node->keys[0]);
        node->keys[0].getData(key2);
        updateNode(nodeMap, node->header.parent, key1, key2);
    } else { // 非子树最小无需向上更新
        // 对于非unique key，可能有多个相同的索引，
        // 如果改大一定是最后一个改大，改小一定是第一个改小
        if (key2 < key1) {
            for (int i = 1; i <= node->header.keyCount; i++) {
                if (key1 == node->keys[i]) {
                    node->keys[i].getData(key2);
                    break;
                }
                // 理论上一定会找到一个等于key1的，所以一定会在此前break
                assert(i <= node->header.keyCount);
            }
        } else {
            for (int i = node->header.keyCount; i >= 1; i--) {
                if (key1 == node->keys[i]) {
                    node->keys[i].getData(key2);
                    break;
                }
                // 理论上一定会找到一个等于key1的，所以一定会在此前break
                assert(i >= 1);
            }
        }
    }
}

BpTreeNode *BpTree::getLNode(unordered_map<PageIndex, BpTreeNode *> &nodeMap, BpTreeNode *node) {
    BpTreeNode *lNode = nullptr;
    if (node->header.leftSibling != 0) {
        lNode = getNode(nodeMap, node->header.leftSibling);
        if (lNode->header.parent != node->header.parent)
            lNode = nullptr;
    }
    return lNode;
}

BpTreeNode *BpTree::getRNode(unordered_map<PageIndex, BpTreeNode *> &nodeMap, BpTreeNode *node) {
    BpTreeNode *rNode = nullptr;
    if (node->header.rightSibling != 0) {
        rNode = getNode(nodeMap, node->header.rightSibling);
        if (rNode->header.parent != node->header.parent)
            rNode = nullptr;
    }
    return rNode;
}

void BpTree::leafBorrowLeft(unordered_map<PageIndex, BpTreeNode *> &nodeMap,
                            BpTreeNode *node, BpTreeNode *lNode) {
    BpKey borrowKey = lNode->keys[lNode->header.keyCount];
    node->insertKey(borrowKey);
    lNode->header.keyCount--;
    // 这里的update理论上只会更新一层
    assert(borrowKey == node->keys[1]);
    updateNode(nodeMap, node->header.parent, node->keys[2], borrowKey);
}

void BpTree::leafBorrowRight(unordered_map<PageIndex, BpTreeNode *> &nodeMap,
                             BpTreeNode *node, BpTreeNode *rNode) {
    BpKey borrowKey = rNode->keys[1];
    node->insertKey(borrowKey);
    rNode->removeKey(borrowKey);
    // 这里也只update一层
    assert(borrowKey == node->keys[node->header.keyCount]);
    updateNode(nodeMap, node->header.parent, borrowKey, rNode->keys[1]);
}

void BpTree::nonleafBorrowLeft(unordered_map<PageIndex, BpTreeNode *> &nodeMap,
                               BpTreeNode *node, BpTreeNode *lNode) {
    BpKey borrowKey = lNode->keys[lNode->header.keyCount];
    node->header.keyCount++;
    // 理论上keys[0]存的就是子树最小值
    for (int i = node->header.keyCount; i >= 1; i--)
        node->keys[i] = node->keys[i - 1];
    node->keys[0] = borrowKey;
    lNode->header.keyCount--;
    // update parent pointer
    auto *son = getNode(nodeMap, borrowKey.position.pageId);
    son->header.parent = node->header.pageIndex;
    // 也只更新一层
    updateNode(nodeMap, node->header.parent, node->keys[1], borrowKey);
}

void BpTree::nonleafBorrowRight(unordered_map<PageIndex, BpTreeNode *> &nodeMap,
                                BpTreeNode *node, BpTreeNode *rNode) {
    BpKey borrowKey = rNode->keys[0];
    node->header.keyCount++;
    node->keys[node->header.keyCount] = borrowKey;
    for (int i = 0; i < rNode->header.keyCount; i++)
        rNode->keys[i] = rNode->keys[i + 1];
    rNode->header.keyCount--;
    // update parent pointer
    auto *son = getNode(nodeMap, borrowKey.position.pageId);
    son->header.parent = node->header.pageIndex;
    // update 1 level
    updateNode(nodeMap, node->header.parent, borrowKey, rNode->keys[0]);
}

void BpTree::leafMerge(unordered_map<PageIndex, BpTreeNode *> &nodeMap,
                       BpTreeNode *lNode, BpTreeNode *rNode) {
    BpKey delKey = rNode->keys[1];
    for (int i = 1; i <= rNode->header.keyCount; i++) {
        lNode->keys[lNode->header.keyCount + i] = rNode->keys[i];
    }
    lNode->header.keyCount += rNode->header.keyCount;
    auto *parent = getNode(nodeMap, lNode->header.parent);
    parent->removeKey(delKey);

    lNode->header.rightSibling = rNode->header.rightSibling;
    if (rNode->header.rightSibling != 0) {
        auto rrNode = getNode(nodeMap, rNode->header.rightSibling);
        rrNode->header.leftSibling = lNode->header.pageIndex;
    }

    // delete rNode
    nodeMap[rNode->header.pageIndex] = nullptr;
    deleteNode(rNode);
}

void BpTree::nonleafMerge(unordered_map<PageIndex, BpTreeNode *> &nodeMap,
                          BpTreeNode *lNode, BpTreeNode *rNode) {
    BpKey delKey = rNode->keys[0];
    for (int i = 0; i <= rNode->header.keyCount; i++) {
        lNode->keys[lNode->header.keyCount + i + 1] = rNode->keys[i];
        // update parent pointer
        auto son = getNode(nodeMap, rNode->keys[i].position.pageId);
        son->header.parent = lNode->header.pageIndex;
    }
    lNode->header.keyCount += rNode->header.keyCount + 1;
    auto *parent = getNode(nodeMap, lNode->header.parent);
    parent->removeKey(delKey);

    lNode->header.rightSibling = rNode->header.rightSibling;
    if (rNode->header.rightSibling != 0) {
        auto rrNode = getNode(nodeMap, rNode->header.rightSibling);
        rrNode->header.leftSibling = lNode->header.pageIndex;
    }

    // delete rNode
    nodeMap[rNode->header.pageIndex] = nullptr;
    deleteNode(rNode);
}

/* ---------- 以下为调试用代码，输出BpTree内容 ---------- */
/* ---------- 以下为调试用代码，输出BpTree内容 ---------- */
/* ---------- 以下为调试用代码，输出BpTree内容 ---------- */

void BpTree::printAllLeaf() {
    auto node = getNode(header.rootPage);
    while (!node->header.isLeaf) {
        auto temp = getNode(node->keys[0].position.pageId);
        delete node;
        node = temp;
    }
    while (true) {
        for (int i = 1; i <= node->header.keyCount; i++) {
            cout << node->keys[i] << endl;
        }
        if (node->header.rightSibling != 0) {
            auto temp = getNode(node->header.rightSibling);
            delete node;
            node = temp;
        } else break;
    }
    delete node;
}

void BpTree::printAllTree() {
    auto node = getNode(header.rootPage);
    while (!node->header.isLeaf) {
        cout << "level" << endl;
        auto node_ = node;
        while (true) {
            cout << node->header.keyCount << "(";
            for (int i = 0; i <= node->header.keyCount; i++) {
                switch (node->header.keyType) {
                    case (KeyType::INT):
                        cout << node->keys[i].dataI << ",";
                        break;
                    case (KeyType::FLOAT):
                        cout << node->keys[i].dataF << ",";
                        break;
                    case (KeyType::STRING):
                        cout << node->keys[i].dataS << ",";
                        break;
                }
                // 检查son-parent指针对
                auto son = getNode(node->keys[i].position.pageId);
                assert(son->header.parent == node->header.pageIndex);
                delete son;
            }
            cout << ") ";
            if (node->header.rightSibling != 0) {
                auto temp = getNode(node->header.rightSibling);
                assert(temp->header.leftSibling == node->header.pageIndex);
                if (node != node_) delete node;
                node = temp;
            } else break;
        }
        cout << endl;
        if (node != node_) delete node;
        node = node_;

        auto temp = getNode(node->keys[0].position.pageId);
        delete node;
        node = temp;
    }
    cout << "leaves:" << endl;
    while (true) {
        cout << node->header.keyCount << "(";
        for (int i = 1; i <= node->header.keyCount; i++) {
            switch (node->header.keyType) {
                case (KeyType::INT):
                    cout << node->keys[i].dataI << ",";
                    break;
                case (KeyType::FLOAT):
                    cout << node->keys[i].dataF << ",";
                    break;
                case (KeyType::STRING):
                    cout << node->keys[i].dataS << ",";
                    break;
            }
        }
        cout << ")";
        if (node->header.rightSibling != 0) {
            auto temp = getNode(node->header.rightSibling);
            delete node;
            node = temp;
        } else break;
    }
    cout << endl;
    delete node;
}

bool BpTree::check() {
    int last = -999;
    auto node = getNode(header.rootPage);
    while (!node->header.isLeaf) {
        auto temp = getNode(node->keys[0].position.pageId);
        delete node;
        node = temp;
    }
    while (true) {
        for (int i = 1; i <= node->header.keyCount; i++) {
            assert(node->keys[i].dataI >= last);
            if (node->keys[i].dataI < last) return false;
            last = node->keys[i].dataI;
            //cout << node->keys[i] << endl;
        }
        if (node->header.rightSibling != 0) {
            auto temp = getNode(node->header.rightSibling);
            delete node;
            node = temp;
        } else break;
    }
    delete node;
    return true;
}

void BpTree::search(const BpKey &equal, vector<Position> &positions) {
    if (equal.type == KeyType::UD) return;
    BpTreeNode *node = getNode(header.rootPage);
    while (!node->header.isLeaf) {
        auto temp = getNode(node->getSonPosition(equal).pageId);
        delete node;
        node = temp;
    }
    while (true) {
        for (int i = 1; i <= node->header.keyCount; i++) {
            if (node->keys[i] == equal)
                positions.push_back(node->keys->position);
            else break;
        }

        if (node->header.rightSibling == 0) break;
        auto temp = getNode(node->header.rightSibling);
        delete node;
        node = temp;
        if (!(node->keys[0] == equal)) break;
    }
    delete node;
}

void BpTree::search(const BpKey &left, const BpKey &right, vector<Position> &positions) {
    if (left.type == KeyType::UD && right.type == KeyType::UD) return;
    else if (left.type == KeyType::UD) {
        auto node = getNode(header.rootPage);
        while (!node->header.isLeaf) {
            auto temp = getNode(node->getSonPosition(right).pageId);
            delete node;
            node = temp;
        }
        while (true) {
            for (int i = 1; i <= node->header.keyCount; i++) {
                if (node->keys[i] <= right)
                    positions.push_back(node->keys->position);
                else break;
            }

            if (node->header.leftSibling == 0) break;
            auto temp = getNode(node->header.leftSibling);
            delete node;
            node = temp;
        }
        delete node;
    } else {
        auto node = getNode(header.rootPage);
        while (!node->header.isLeaf) {
            auto temp = getNode(node->getSonPosition(left).pageId);
            delete node;
            node = temp;
        }
        while (true) {
            for (int i = 1; i <= node->header.keyCount; i++) {
                if (left <= node->keys[i])
                    positions.push_back(node->keys->position);
                else break;
            }

            if (node->header.rightSibling == 0) break;
            auto temp = getNode(node->header.leftSibling);
            delete node;
            node = temp;
            // 有右边界，并且超了
            if (right.type != KeyType::UD && right < node->keys[0])
                break;
        }
        delete node;
    }
}
