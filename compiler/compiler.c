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

#define OPCODE_SLOTS(opCode, effect) effect,  
static const int opCodeSlotsUsed[] = {
   #include "opcode.inc"
};
#undef OPCODE_SLOTS

static void initCompileUnit(Parser* parser, CompileUnit* cu, CompileUnit* enclosingUnit, bool isMethod) {
    parser->curCompileUnit = cu;
    cu->curParser = parser;
    cu->enclosingUnit = enclosingUnit;
    cu->curLoop = NULL;
    cu->enclosingClassBK = NULL;

    if (enclosingUnit == NULL) {
        cu->scopeDepth = -1;
        cu->localVarNum = 0;
    } else {  // local scope
        if (isMethod) {  // class's method
            cu->localVars[0].name = "this"; 
            cu->localVars[0].length = 4; 
        } else {	  
            cu->localVars[0].name = NULL; 
            cu->localVars[0].length = 0; 
        }
        cu->localVars[0].scopeDepth = -1; 
        cu->localVars[0].isUpvalue = false; 
        cu->localVarNum = 1; 
        cu->scopeDepth = 0; 
    }
    cu->stackSlotNum = cu->localVarNum;
    cu->fn = newObjFn(cu->curParser->vm, cu->curParser->curModule, cu->localVarNum);
}

static int writeByte(CompileUnit* cu, int byte) {
#if DEBUG
    IntBufferAdd(cu->curParser->vm, &cu->fn->debug->lineNo, cu->curParser->preToken.lineNo);
#endif
    ByteBufferAdd(cu->curParser->vm, &cu->fn->instrStream, (uint8_t)byte);
    return cu->fn->instrStream.count - 1;
}

static void writeOpCode(CompileUnit* cu, OpCode opCode) {
    writeByte(cu, opCode);
    cu->stackSlotNum += opCodeSlotsUsed[opCode];
    if (cu->stackSlotNum > cu->fn->maxStackSlotUsedNum) {
        cu->fn->maxStackSlotUsedNum = cu->stackSlotNum;
    }
}

static int writeByteOperand(CompileUnit* cu, int operand) {
   return writeByte(cu, operand);
}

inline static void writeShortOperand(CompileUnit* cu, int operand) {
   writeByte(cu, (operand >> 8) & 0xff); 
   writeByte(cu, operand & 0xff);        
}

static int writeOpCodeByteOperand(CompileUnit* cu, OpCode opCode, int operand) {
   writeOpCode(cu, opCode);
   return writeByteOperand(cu, operand);
}

static void writeOpCodeShortOperand(CompileUnit* cu, OpCode opCode, int operand) {
   writeOpCode(cu, opCode);
   writeShortOperand(cu, operand);
}

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

static void compileProgram(CompileUnit* cu) {
   ;
}

ObjFn* compileModule(VM* vm, ObjModule* objModule, const char* moduleCode) {
    Parser parser;
    parser.parent = vm->curParser;
    vm->curParser = &parser;

    if (objModule->name == NULL) {
        initParser(vm, &parser, "core.script.inc", moduleCode, objModule);
    } else {
        initParser(vm, &parser, (const char*)objModule->name->value.start, moduleCode, objModule);
    }

    CompileUnit moduleCU;
    initCompileUnit(&parser, &moduleCU, NULL, false);

    uint32_t moduleVarNumBefor = objModule->moduleVarValue.count;
    getNextToken(&parser);

    while (!matchToken(&parser, TOKEN_EOF)) {
        compileProgram(&moduleCU);
    }

    // ! not finished yet, UNREACHABLE!!!
    printf("There is something to do...\n"); 
    exit(0);
}
