#ifndef _OBJECT_FN_H
#define _OBJECT_FN_H
#include "utils.h"
#include "obj_meta.h"

typedef struct {
   char* fnName;    
   IntBuffer lineNo; 
} FnDebug;   // debug struct in function

typedef struct {
   ObjHeader objHeader;
   ByteBuffer instrStream; // instruction stream after compilation
   ValueBuffer constants;  // constant table in function
  
   ObjModule* module; // belong to xxx module

   uint32_t maxStackSlotUsedNum;
   uint32_t upvalueNum;	 
   uint8_t argNum; 
#if DEBUG
   FnDebug* debug;
#endif
} ObjFn;   

typedef struct upvalue {
   ObjHeader objHeader;
   Value* localVarPtr;
   Value closedUpvalue;
   struct upvalue* next;   
} ObjUpvalue;

typedef struct {
   ObjHeader objHeader;
   ObjFn* fn;   
   ObjUpvalue* upvalues[0]; // for close upvalue
} ObjClosure;

// function context
typedef struct {
   uint8_t* ip;    
   ObjClosure* closure;
   //frame share thread.stack
   Value* stackStart;//point to this frame's thread.stack addr 
} Frame;

#define INITIAL_FRAME_NUM 4

ObjUpvalue* newObjUpvalue(VM* vm, Value* localVarPtr);
ObjClosure* newObjClosure(VM* vm, ObjFn* objFn);
ObjFn* newObjFn(VM* vm, ObjModule* objModule, uint32_t maxStackSlotUsedNum);

#endif