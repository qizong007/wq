#include "obj_header.h"
#include "class.h"
#include "vm.h"

DEFINE_BUFFER_METHOD(Value)

void initObjHeader(VM* vm, ObjHeader* objHeader, ObjType objType, Class* class){
    objHeader->type = objType;
    objHeader->isDark = false;
    objHeader->class = class;
    // head insertion
    objHeader->next = vm->allObjects;
    vm->allObjects = objHeader;
}