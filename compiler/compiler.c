#include "compiler.h"
#include "parser.h"
#include "core.h"
#include <string.h>
#if DEBUG
   #include "debug.h"
#endif

struct compileUnit {
    ObjFn* fn;
    LocalVar localVars[MAX_LOCAL_VAR_NUM];
    uint32_t localVarNum;
    Upvalue upvalues[MAX_UPVALUE_NUM];
    int scopeDepth; // -1:module  0:outer  >=1:inner
    uint32_t stackSlotNum;
    Loop* curLoop;
    ClassBookKeep* enclosingClassBK;
    struct compileUnit* enclosingUnit;
    Parser* curParser;
}; 

// define a var [name] = [value], in objModule
int defineModuleVar(VM* vm, ObjModule* objModule, const char* name, uint32_t length, Value value) {
    if (length > MAX_ID_LEN) {
        char id[MAX_ID_LEN] = {'\0'};
        memcpy(id, name, length);
        if (vm->curParser != NULL) {   // compiling source file
            COMPILE_ERROR(vm->curParser, "length of identifier \"%s\" should be no more than %d", id, MAX_ID_LEN);
        } else {   // called before compiling(eg. loading modules) 
            MEM_ERROR("length of identifier \"%s\" should be no more than %d", id, MAX_ID_LEN);
        }
    }

    int symbolIndex = getIndexFromSymbolTable(&objModule->moduleVarName, name, length);
    if (symbolIndex == -1) {  
        symbolIndex = addSymbol(vm, &objModule->moduleVarName, name, length);
        ValueBufferAdd(vm, &objModule->moduleVarValue, value);
    } else if (VALUE_IS_NUM(objModule->moduleVarValue.datas[symbolIndex])) {
        // module var is referred but not defined
        objModule->moduleVarValue.datas[symbolIndex] = value; 
    } else {
        // re-definition
        symbolIndex = -1;  
    }

    return symbolIndex;
}

// TODO
ObjFn* compileModule(VM* vm, ObjModule* objModule, const char* moduleCode) {
   ;
}
