#ifndef _OBJECT_HEADER_H
#define _OBJECT_HEADER_H
#include "utils.h"

// object type
typedef enum {
    OT_CLASS,  // for class, the rest is for object   
    OT_LIST,
    OT_MAP,
    OT_MODULE,
    OT_RANGE,
    OT_STRING,
    OT_UPVALUE,
    OT_FUNCTION,
    OT_CLOSURE,
    OT_INSTANCE,
    OT_THREAD
} ObjType;  

typedef struct objHeader {
    ObjType type;
    bool isDark;	   // is object could be arrived
    Class* class;   
    struct objHeader* next;   // object linked-list, it's good for GC use
} ObjHeader;

typedef enum {
    VT_UNDEFINED,
    VT_NULL,
    VT_FALSE,
    VT_TRUE,
    VT_NUM,
    VT_OBJ   // value type is object, point to the object header
} ValueType; 

typedef struct {
    ValueType type;
    union {
        double num;
        ObjHeader* objHeader;
    };
} Value;

DECLARE_BUFFER_TYPE(Value)

void initObjHeader(VM* vm, ObjHeader* objHeader, ObjType objType, Class* class);

#endif