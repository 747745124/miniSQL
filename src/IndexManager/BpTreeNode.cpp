#include "BpTreeNode.h"
#include "../BufferManager/page.h"
#include <stdexcept>
#include <iostream>

BpTreeNode::BpTreeNode(const BpTreeNode &node) {
    this->header = node.header;
    if (keys == nullptr) keys = new BpKey[MAXKEYS];
    for (int i = 0; i < this->header.keyCount; i++) {
        this->keys[i] = node.keys[i];
    }
}

BpTreeNode::BpTreeNode() {
    this->header.pageIndex = -1;
    this->header.keyCount = 0;
    this->header.keyType = KeyType::UD;
    header.parent = header.leftSibling = header.rightSibling = 0;

    this->keys = new BpKey[MAXKEYS];
}

BpTreeNode &BpTreeNode::operator=(const BpTreeNode &node) {
    if (this != &node) {
        this->header = node.header;
        if (node.keys != nullptr) {
            if (keys == nullptr) keys = new BpKey[MAXKEYS];
            for (int i = 0; i < this->header.keyCount; i++) {
                this->keys[i] = node.keys[i];
            }
        }
    }
    return *this;
}


int BpTreeNode::getRawSize() const {
    return (int) sizeof(BpTreeNodeHeader) + header.keyCount * header.keySize;
}

bool BpTreeNode::isOverflow() const {
    return header.keyCount > maxKeyCount();
}

bool BpTreeNode::isUnderflow() const {
    return header.keyCount < maxKeyCount() / 2;
}

bool BpTreeNode::hasSpare() const {
    return header.keyCount > maxKeyCount() / 2;
}

bool BpTreeNode::insertKey(const BpKey &key) {
    // check key type
    if (key.type == KeyType::UD || key.type != this->header.keyType) {
#ifdef DEBUG
        cout << (int)key.type << ' ' << (int)this->header.keyType << endl;
#endif
        throw invalid_argument("Invalid key type.");
        // return false;
    }
    // find insert position
    int insPos;
    for (insPos = 1; insPos <= header.keyCount; insPos++) {
        if (key < keys[insPos]) break;
    }
    // insert
    for (int i = header.keyCount + 1; i > insPos; i--) {
        keys[i] = keys[i - 1];
    }
    keys[insPos] = key;
    header.keyCount++;
    return true;
}

bool BpTreeNode::removeKey(const BpKey &key) {
    if (key.type == KeyType::UD || key.type != header.keyType) {
        throw invalid_argument("Invalid key type.");
    }
    // find remove position
    int delPos;
    for (delPos = 1; delPos <= header.keyCount; delPos++) {
        if (key == keys[delPos]) break;
    }
    if (delPos > header.keyCount) return false;

    for (int i = delPos; i <= header.keyCount; i++) {
        keys[i] = keys[i + 1];
    }
    header.keyCount--;
    return true;
}

Position BpTreeNode::getPosition(const BpKey &key) const {
    if (key.type == KeyType::UD || key.type != header.keyType) {
        throw invalid_argument("Invalid key type.");
    }
    for (int i = 1; i <= header.keyCount; i++) {
        if (key == keys[i]) return keys[i].position;
    }
    return {-1, -1};
}

Position BpTreeNode::getSonPosition(const BpKey &key) const {
    if (key.type == KeyType::UD || key.type != header.keyType) {
        throw invalid_argument("Invalid key type.");
    }

    for (int i = 1; i <= header.keyCount; i++) {
        if (key < keys[i]) return keys[i - 1].position;
    }
    return keys[header.keyCount].position;
}

void BpTreeNode::writeToData(Byte *data) const {
    memcpy(data, &header, sizeof(header));
    data += sizeof(header);
    for (int i = 0; i <= header.keyCount; i++) {
        data = keys[i].writeToData(data);
    }
}

void BpTreeNode::convertFromData(Byte *data) {
    memcpy(&header, data, sizeof(header));
    data += sizeof(header);
    for (int i = 0; i <= header.keyCount; i++) {
        data = keys[i].convertFromData(data);
    }
}

// maxKeyCount()+1 阶B+树
int BpTreeNode::maxKeyCount() const {
    if (header.keySize <= 0) return MAXKEYS;
    return (PAGESIZE - (int) sizeof(header)) / header.keySize - 1;
}

