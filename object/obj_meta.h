#ifndef _OBJECT_META_OBJ_H
#define _OBJECT_META_OBJ_H
#include "obj_string.h"

typedef struct {
   ObjHeader objHeader;
   SymbolTable moduleVarName;   
   ValueBuffer moduleVarValue;  
   ObjString* name;   
} ObjModule;   

typedef struct {
   ObjHeader objHeader;
   Value fields[0]; // flexible array
} ObjInstance;	

ObjModule* newObjModule(VM* vm, const char* modName);
ObjInstance* newObjInstance(VM* vm, Class* class);

#endif