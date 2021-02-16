#ifndef _COMPILER_COMPILER_H
#define _COMPILER_COMPILER_H
#include "obj_fn.h"

#define MAX_LOCAL_VAR_NUM 128
#define MAX_UPVALUE_NUM 128
#define MAX_ID_LEN 128  

#define MAX_METHOD_NAME_LEN MAX_ID_LEN
#define MAX_ARG_NUM 16

// func name len + '(' + n [args] + (n-1)[','] + ')'
#define MAX_SIGN_LEN MAX_METHOD_NAME_LEN + MAX_ARG_NUM * 2 + 1

#define MAX_FIELD_NUM 128

typedef struct {
   bool isEnclosingLocalVar;   
   uint32_t index;
} Upvalue;  

typedef struct {
   const char* name; 
   uint32_t length;
   int scopeDepth;  
   bool isUpvalue;
} LocalVar;    

typedef enum {
   SIGN_CONSTRUCT,  
   SIGN_METHOD, 
   SIGN_GETTER, 
   SIGN_SETTER, 
   SIGN_SUBSCRIPT, 
   SIGN_SUBSCRIPT_SETTER   
} SignatureType;   

typedef struct {
   SignatureType type;  
   const char* name;	
   uint32_t length;	
   uint32_t argNum;	
} Signature;	

typedef struct loop {
   int condStartIndex;   
   int bodyStartIndex;  
   int scopeDepth;  
   int exitIndex;   
   struct loop* enclosingLoop;   // outer loop 
} Loop;  

typedef struct {
   ObjString* name;	      
   SymbolTable fields;	   
   bool inStatic;	    
   IntBuffer instantMethods;  // index(in VM->allMethodNames)
   IntBuffer staticMethods;   
   Signature* signature;      // current compiling 
} ClassBookKeep;    // record class info when compiling

typedef struct compileUnit CompileUnit;// in compiler.c

int defineModuleVar(VM* vm, ObjModule* objModule, const char* name, uint32_t length, Value value);
ObjFn* compileModule(VM* vm, ObjModule* objModule, const char* moduleCode);

#endif
