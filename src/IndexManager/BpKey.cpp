#include "BpKey.h"
#include <cstring>
#include <cassert>
#include <iostream>
#include <string>

using namespace std;

BpKey::BpKey() : type(KeyType::UD) {}

BpKey::~BpKey() {
    delete[] dataS;
}

BpKey::BpKey(const BpKey &key) {
    type = key.type;
    position = key.position;
    dataI = key.dataI;
    dataF = key.dataF;
    if (type == KeyType::STRING) {
        if (dataS == nullptr || strLen != key.strLen) {
            assert(key.dataS != nullptr);
            delete dataS;
            dataS = new char[key.strLen + 1]{0};
        }
        memcpy(dataS, key.dataS, key.strLen);
    }
    strLen = key.strLen;
}

BpKey::BpKey(const Data &data, Position position) {
    this->position = position;

    if (data.type == -1) {
        type = KeyType::INT;
        dataI = data.datai;
    } else if (data.type == 0) {
        type = KeyType::FLOAT;
        dataF = data.dataf;
    } else if (data.type >= 1 && data.type <= 255) {
        type = KeyType::STRING;
        strLen = data.type;
        dataS = new char[strLen + 1]{};
        sprintf(dataS, "%s", data.datas.c_str());
    } else type = KeyType::UD;
}

BpKey &BpKey::operator=(const BpKey &key) {
    if (this == &key) return *this;
    type = key.type;
    position = key.position;
    dataI = key.dataI;
    dataF = key.dataF;
    if (type == KeyType::STRING) {
        if (dataS == nullptr || strLen != key.strLen) {
            assert(key.dataS != nullptr);
            delete dataS;
            dataS = new char[key.strLen + 1]{0};
        }
        memcpy(dataS, key.dataS, key.strLen);
    }
    strLen = key.strLen;
    return *this;
}

int BpKey::getRawSize(KeyType keyType, int strLen = 0) {
    int dataSize;
    if (keyType == KeyType::INT) {
        dataSize = sizeof(int);
    } else if (keyType == KeyType::FLOAT) {
        dataSize = sizeof(double);
    } else if (keyType == KeyType::STRING) {
        dataSize = strLen + (int) sizeof(strLen);
    } else { // Undefined
        return -1;
    }

    return dataSize + (int) sizeof(Position) + (int) sizeof(KeyType);
}

int BpKey::getRawSize() const {
    return getRawSize(type, strLen);
}

bool operator<(const BpKey &key1, const BpKey &key2) {
    if (key1.type == key2.type) {
        switch (key1.type) {
            case KeyType::INT:
                return key1.dataI < key2.dataI;
                break;
            case KeyType::FLOAT:
                return key1.dataF < key2.dataF;
                break;
            case KeyType::STRING:
                return (strcmp(key1.dataS, key2.dataS) < 0);
                break;
            default:
                return false;
                break;
        }
    } else {
        return key1.type < key2.type;
    }
}

bool operator==(const BpKey &key1, const BpKey &key2) {
    if (key1.type == key2.type) {
        switch (key1.type) {
            case KeyType::INT:
                return key1.dataI == key2.dataI;
                break;
            case KeyType::FLOAT:
                return key1.dataF == key2.dataF;
                break;
            case KeyType::STRING:
                return (strcmp(key1.dataS, key2.dataS) == 0);
                break;
            default:
                return true;
                break;
        }
    } else {
        return false;
    }
}

bool operator<=(const BpKey &key1, const BpKey &key2) {
    return key1 < key2 || key1 == key2;
}

Byte *BpKey::convertFromData(Byte *data) {
    memcpy(&position, data, sizeof(Position));
    data += sizeof(Position);
    memcpy(&type, data, sizeof(KeyType));
    data += sizeof(KeyType);
    if (type == KeyType::STRING) {
        memcpy(&strLen, data, sizeof(strLen));
        data += sizeof(strLen);
    }
    switch (type) {
        case KeyType::INT:
            memcpy(&dataI, data, sizeof(int));
            data += sizeof(int);
            break;
        case KeyType::FLOAT:
            memcpy(&dataF, data, sizeof(double));
            data += sizeof(double);
            break;
        case KeyType::STRING:
            delete dataS;
            dataS = new char[strLen + 1]{0};
            memcpy(dataS, data, strLen);
            data += strLen;
            break;
        default:
            break;
    }
    return data;
}

Byte *BpKey::writeToData(Byte *data) const {
    memcpy(data, &position, sizeof(Position));
    data += sizeof(Position);
    memcpy(data, &type, sizeof(KeyType));
    data += sizeof(KeyType);
    if (type == KeyType::STRING) {
        memcpy(data, &strLen, sizeof(strLen));
        data += sizeof(strLen);
    }

    switch (type) {
        case KeyType::INT:
            memcpy(data, &dataI, sizeof(int));
            data += sizeof(int);
            break;
        case KeyType::FLOAT:
            memcpy(data, &dataF, sizeof(double));
            data += sizeof(double);
            break;
        case KeyType::STRING:
            memcpy(data, dataS, strLen);
            data += strLen;
            break;
        default:
            break;
    }
    return data;
}

void BpKey::getData(const BpKey &key) {
    assert(this->type == key.type);
    switch (type) {
        case KeyType::INT:
            this->dataI = key.dataI;
            break;
        case KeyType::FLOAT:
            this->dataF = key.dataF;
            break;
        case KeyType::STRING:
            memcpy(this->dataS, key.dataS, strLen);
            break;
        default:
            break;
    }
}

ostream &operator<<(ostream &out, BpKey &key) {
    switch (key.type) {
        case KeyType::INT:
            out << key.dataI;
            break;
        case KeyType::FLOAT:
            out << key.dataF;
            break;
        case KeyType::STRING:
            assert(key.dataS != nullptr);
            std::string str(key.dataS);
            out << str;
            break;
    }
    out << ' ' << key.position.pageId << ' ' << key.position.offset;
    return out;
}

