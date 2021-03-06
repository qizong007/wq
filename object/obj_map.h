#ifndef _OBJECT_MAP_H
#define _OBJECT_MAP_H
#include "obj_header.h"

#define MAP_LOAD_FACTOR 0.8

typedef struct {  
   Value key; 
   Value value;
} Entry; // k-v pair

typedef struct {
   ObjHeader objHeader;
   uint32_t capacity; // total
   uint32_t count;    // used
   Entry* entries;    // Entry array
} ObjMap;

ObjMap* newObjMap(VM* vm);

void mapSet(VM* vm, ObjMap* objMap, Value key, Value value);
Value mapGet(ObjMap* objMap, Value key);
void clearMap(VM* vm, ObjMap* objMap);
Value removeKey(VM* vm, ObjMap* objMap, Value key);

#endif
