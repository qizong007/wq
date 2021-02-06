#include "obj_fn.h"
#include "obj_meta.h"
#include "class.h"
#include "vm.h"
#include <string.h>

ObjModule* newObjModule(VM* vm, const char* modName) {
   ObjModule* objModule = ALLOCATE(vm, ObjModule);
   if (objModule == NULL) {
      MEM_ERROR("allocate ObjModule failed!"); 
   }
   //ObjModule is a meta info object, not belong to any class
   initObjHeader(vm, &objModule->objHeader, OT_MODULE, NULL);

   StringBufferInit(&objModule->moduleVarName);
   ValueBufferInit(&objModule->moduleVarValue);

   objModule->name = NULL;   // core module named NULL
   if (modName != NULL) {
      objModule->name = newObjString(vm, modName, strlen(modName));
   }
   return objModule;
}

ObjInstance* newObjInstance(VM* vm, Class* class) {
    ObjInstance* objInstance = ALLOCATE_EXTRA(vm, ObjInstance, sizeof(Value) * class->fieldNum);
    initObjHeader(vm, &objInstance->objHeader, OT_INSTANCE, class);
    uint32_t index = 0;
    while (index < class->fieldNum) {
        objInstance->fields[index] = VT_TO_VALUE(VT_NULL);
    }
    return objInstance;
}