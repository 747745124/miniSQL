#ifndef BASIC_H
#define BASIC_H

#include "include/table.h"
#include "include/compare.h"
#include "include/tuple.h"
#include "include/position.h"

const int BUFFER_POOL_SIZE = 8192 * 4;

typedef int PageIndex;
typedef unsigned char Byte;

#define DEBUG
//#define TIME
#endif