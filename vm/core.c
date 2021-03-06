#include "core.h"
#include <string.h>
#include <sys/stat.h>
#include "utils.h"
#include "vm.h"
#include "obj_thread.h"
#include "compiler.h"

char* rootDir = NULL;

#define CORE_MODULE VT_TO_VALUE(VT_NULL)

// return valus's type is Value
// it will be put at args[0], args is a Value array
#define RET_VALUE(value)\
    do {\
        args[0] = value;\
        return true;\
    } while(0);

#define RET_OBJ(objPtr) RET_VALUE(OBJ_TO_VALUE(objPtr))
#define RET_BOOL(boolean) RET_VALUE(BOOL_TO_VALUE(boolean))
#define RET_NUM(num) RET_VALUE(NUM_TO_VALUE(num))
#define RET_NULL RET_VALUE(VT_TO_VALUE(VT_NULL))
#define RET_TRUE RET_VALUE(VT_TO_VALUE(VT_TRUE))
#define RET_FALSE RET_VALUE(VT_TO_VALUE(VT_FALSE))

// set thread error
#define SET_ERROR_FALSE(vmPtr, errMsg) \
    do {\
        vmPtr->curThread->errorObj = OBJ_TO_VALUE(newObjString(vmPtr, errMsg, strlen(errMsg)));\
        return false;\
    } while(0);

#define PRIM_METHOD_BIND(classPtr, methodName, func) {\
    uint32_t length = strlen(methodName);\
    int globalIdx = getIndexFromSymbolTable(&vm->allMethodNames, methodName, length);\
    if (globalIdx == -1) {\
        globalIdx = addSymbol(vm, &vm->allMethodNames, methodName, length);\
    }\
    Method method;\
    method.type = MT_PRIMITIVE;\
    method.primFn = func;\
    bindMethod(vm, classPtr, (uint32_t)globalIdx, method);\
}

static bool primObjectNot(VM* vm UNUSED, Value* args) {
    RET_VALUE(VT_TO_VALUE(VT_FALSE));
}

// args[0] == args[1]
static bool primObjectEqual(VM* vm UNUSED, Value* args) {
    Value boolValue = BOOL_TO_VALUE(valueIsEqual(args[0], args[1]));
    RET_VALUE(boolValue);
}

// args[0] != args[1]
static bool primObjectNotEqual(VM* vm UNUSED, Value* args) {
    Value boolValue = BOOL_TO_VALUE(!valueIsEqual(args[0], args[1]));
    RET_VALUE(boolValue);
}

// IS-A: args[0]'s class extends args[1]'s class
static bool primObjectIs(VM* vm, Value* args) {
    if (!VALUE_IS_CLASS(args[1])) {
        RUN_ERROR("argument must be class!");
    }
    
    Class* thisClass = getClassOfObj(vm, args[0]);
    Class* baseClass = (Class*)(args[1].objHeader);

    // maybe multi-level inheritance
    while (baseClass != NULL) {
        if (thisClass == baseClass) {
            RET_VALUE(VT_TO_VALUE(VT_TRUE));
        }
        baseClass = baseClass->superClass;
    }
    RET_VALUE(VT_TO_VALUE(VT_FALSE));
}

// args[0].tostring: return args[0]'s class name
static bool primObjectToString(VM* vm UNUSED, Value* args) {
    Class* class = args[0].objHeader->class;
    Value nameValue = OBJ_TO_VALUE(class->name);
    RET_VALUE(nameValue);
}

// args[0].type: return args[0]'s class type
static bool primObjectType(VM* vm, Value* args) {
    Class* class = getClassOfObj(vm, args[0]);
    RET_OBJ(class);
}

// args[0].name: return class name
static bool primClassName(VM* vm UNUSED, Value* args) {
   RET_OBJ(VALUE_TO_CLASS(args[0])->name);
}

// args[0].supertype: return args[0]'s super class
static bool primClassSupertype(VM* vm UNUSED, Value* args) {
    Class* class = VALUE_TO_CLASS(args[0]);
    if (class->superClass != NULL) {
        RET_OBJ(class->superClass);
    } 
    RET_VALUE(VT_TO_VALUE(VT_NULL));
}

//args[0].toString: return class name
static bool primClassToString(VM* vm UNUSED, Value* args) {
    RET_OBJ(VALUE_TO_CLASS(args[0])->name);
}

// args[0].same(args[1], args[2])
static bool primObjectmetaSame(VM* vm UNUSED, Value* args) {
    Value boolValue = BOOL_TO_VALUE(valueIsEqual(args[1], args[2]));
    RET_VALUE(boolValue);
}

// find symbol in table, return index
int getIndexFromSymbolTable(SymbolTable* table, const char* symbol, uint32_t length) {
    ASSERT(length != 0, "length of symbol is 0!");
    uint32_t index = 0;
    while (index < table->count) {
        if (length == table->datas[index].length && memcmp(table->datas[index].str, symbol, length) == 0) {
            return index;
        }
        index++;
    }
    return -1;
}

// add symbol to table, return index
int addSymbol(VM* vm, SymbolTable* table, const char* symbol, uint32_t length) {
    ASSERT(length != 0, "length of symbol is 0!");
    String string;
    string.str = ALLOCATE_ARRAY(vm, char, length + 1);
    memcpy(string.str, symbol, length);
    string.str[length] = '\0';
    string.length = length;
    StringBufferAdd(vm, table, string);
    return table->count - 1;
}

static Class* defineClass(VM* vm, ObjModule* objModule, const char* name) {
    Class* class = newRawClass(vm, name, 0);
    defineModuleVar(vm, objModule, name, strlen(name), OBJ_TO_VALUE(class));
    return class;
}

void bindMethod(VM* vm, Class* class, uint32_t index, Method method) {
    if (index >= class->methods.count) {
        Method emptyPad = {MT_NONE, {0}};
        MethodBufferFillWrite(vm, &class->methods, emptyPad, index - class->methods.count + 1); 
    }
    class->methods.datas[index] = method;
}

void bindSuperClass(VM* vm, Class* subClass, Class* superClass) {
    subClass->superClass = superClass;
    subClass->fieldNum += superClass->fieldNum;
    uint32_t idx = 0;
    while (idx < superClass->methods.count) {
        bindMethod(vm, subClass, idx, superClass->methods.datas[idx]); 
        idx++;
    }
}

static ObjModule* getModule(VM* vm, Value moduleName) {
    Value value = mapGet(vm->allModules, moduleName);
    if (value.type == VT_UNDEFINED) {
        return NULL;
    }  
    return VALUE_TO_OBJMODULE(value);
}

static ObjThread* loadModule(VM* vm, Value moduleName, const char* moduleCode) {
    /* Load */
    ObjModule* module = getModule(vm, moduleName);
    // make sure the module is ready
    if (module == NULL) {
        ObjString* modName = VALUE_TO_OBJSTR(moduleName);
        ASSERT(modName->value.start[modName->value.length] == '\0', "string.value.start is not terminated!");
        module = newObjModule(vm, modName->value.start);
        mapSet(vm, vm->allModules, moduleName, OBJ_TO_VALUE(module));     
        // extends core modules vars
        ObjModule* coreModule = getModule(vm, CORE_MODULE);
        uint32_t idx = 0;
        while (idx < coreModule->moduleVarName.count) {
            defineModuleVar(vm, module,coreModule->moduleVarName.datas[idx].str,
                            strlen(coreModule->moduleVarName.datas[idx].str),
                            coreModule->moduleVarValue.datas[idx]);
            idx++; 
        }
    }

    /* Compile */
    ObjFn* fn = compileModule(vm, module, moduleCode);
    ObjClosure* objClosure = newObjClosure(vm, fn);
    ObjThread* moduleThread = newObjThread(vm, objClosure);

    return moduleThread;  
}

// TODO
VMResult executeModule(VM* vm, Value moduleName, const char* moduleCode) {
    ObjThread* objThread = loadModule(vm, moduleName, moduleCode);
    return VM_RESULT_ERROR;
}

// compile core module
void buildCore(VM* vm) {
    ObjModule* coreModule = newObjModule(vm, NULL);
    mapSet(vm, vm->allModules, CORE_MODULE, OBJ_TO_VALUE(coreModule));

    // create object class, and bind methods
    vm->objectClass = defineClass(vm, coreModule, "object");
    PRIM_METHOD_BIND(vm->objectClass, "!", primObjectNot);
    PRIM_METHOD_BIND(vm->objectClass, "==(_)", primObjectEqual);
    PRIM_METHOD_BIND(vm->objectClass, "!=(_)", primObjectNotEqual);
    PRIM_METHOD_BIND(vm->objectClass, "is(_)", primObjectIs);
    PRIM_METHOD_BIND(vm->objectClass, "toString", primObjectToString);
    PRIM_METHOD_BIND(vm->objectClass, "type", primObjectType);

    // define meta class's meta class
    vm->classOfClass = defineClass(vm, coreModule, "class");
    // bind 'class' to 'object'(object is the God)
    bindSuperClass(vm, vm->classOfClass, vm->objectClass);

    PRIM_METHOD_BIND(vm->classOfClass, "name", primClassName);
    PRIM_METHOD_BIND(vm->classOfClass, "supertype", primClassSupertype);
    PRIM_METHOD_BIND(vm->classOfClass, "toString", primClassToString);

    Class* objectMetaclass = defineClass(vm, coreModule, "objectMeta");
    bindSuperClass(vm, objectMetaclass, vm->classOfClass);
    PRIM_METHOD_BIND(objectMetaclass, "same(_,_)", primObjectmetaSame);

    // bind to meta
    vm->objectClass->objHeader.class = objectMetaclass;
    vm->classOfClass->objHeader.class = vm->classOfClass; // close-loop
    objectMetaclass->objHeader.class = vm->classOfClass;

    executeModule(vm, CORE_MODULE, coreModuleCode);
}

char* readFile(const char* path){
    FILE* file = fopen(path, "r");
    if(file == NULL){
        IO_ERROR("Couldn't open file \"%s\".\n", path);
    }

    struct stat fileStat;
    stat(path, &fileStat);
    size_t fileSize = fileStat.st_size;
    char* fileContent = (char*)malloc(fileSize + 1);
    if(fileContent == NULL){
        MEM_ERROR("Couldn't allocate memory for reading file \"%s\".\n", path);
    }

    size_t numRead = fread(fileContent, sizeof(char), fileSize, file);
    if(numRead < fileSize){
        IO_ERROR("Couldn't read file \"%s\".\n", path);
    }
    fileContent[fileSize] = '\0';

    fclose(file);
    return fileContent;
} 
