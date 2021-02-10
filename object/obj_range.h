#ifndef _OBJECT_RANGE_H
#define _OBJECT_RANGE_H
#include "class.h"

typedef struct {
   ObjHeader objHeader;
   int from;   // start
   int to;     // end
} ObjRange;   

ObjRange* newObjRange(VM* vm, int from, int to);

#endif