#ifndef _POSITION_H_
#define _POSITION_H_

class Position {
public:
    int pageId; //pageID
    int offset; //ๅ็งป้

    Position(int pageId1 = -1, int offset1 = -1) : pageId(pageId1), offset(offset1) {}
};

#endif