/* implement the Virtual Machine */
#ifndef _VM_VM_H
#define _VM_VM_H
#include "common.h"
#include "class.h"
#include "obj_map.h"
#include "obj_thread.h"
#include "parser.h"

#define OPCODE_SLOTS(opcode, effect) OPCODE_##opcode,
typedef enum {
   #include "opcode.inc"
} OpCode;
#undef OPCODE_SLOTS

typedef enum vmResult {
    VM_RESULT_SUCCESS,
    VM_RESULT_ERROR
} VMResult; 
// if there's no RE, we can store the char code to file cache
// avoid to compile again

struct vm {
    Class* classOfClass;
    Class* objectClass;
    Class* stringClass;
    Class* mapClass;
    Class* rangeClass;
    Class* listClass;
    Class* nullClass;
    Class* boolClass;
    Class* numClass;
    Class* fnClass;
    Class* threadClass;
    uint32_t allocatedBytes;
    ObjHeader* allObjects; // all allocated objects
    SymbolTable allMethodNames;
    ObjMap* allModules;
    ObjThread* curThread;  //current thread
    Parser* curParser;     // current parser
};

void initVM(VM* vm);
VM* newVM(void);

#endif