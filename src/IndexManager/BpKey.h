//
// Created by 93209 on 2021/6/2.
//

#ifndef MINISQL_BPKEY_H
#define MINISQL_BPKEY_H

#include "../basic.h"


enum class KeyType {
    INT, FLOAT, STRING, UD
};

class BpKey {
public:
    KeyType type;
    int dataI{};
    double dataF{};
    char *dataS = nullptr;
    int strLen{};
    Position position{};

    BpKey();

    explicit BpKey(const Data &data, Position position = Position(-1, -1));

    BpKey(const BpKey &key);

    ~BpKey();

    static int getRawSize(KeyType keyType, int strLen);

    int getRawSize() const;

    Byte *convertFromData(Byte *data);

    Byte *writeToData(Byte *data) const;

    void getData(const BpKey &key);

    friend bool operator<(const BpKey &key1, const BpKey &key2);

    friend bool operator==(const BpKey &key1, const BpKey &key2);

    friend bool operator<=(const BpKey &key1, const BpKey &key2);

    friend ostream &operator<<(ostream &out, BpKey &key);

    BpKey &operator=(const BpKey &key);
};


#endif //MINISQL_BPKEY_H
