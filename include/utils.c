#include "utils.h"
#include "vm.h"
#include "parser.h"
#include <stdlib.h>
#include <stdarg.h>

void* memManager(VM* vm, void* ptr, uint32_t oldSize, uint32_t newSize) {
    vm->allocatedBytes += newSize - oldSize;
    // avoid realloc(NULL,0) define a new address
    if(newSize == 0){
        free(ptr);
        return NULL;
    }
    /*
    * [ Memory Expansion ]
    * void* realloc(void *mem_address, unsigned int newsize):
    * (1) If mem_address has enough continuous space -> expand the space, 
    *     and then return the mem_address.
    * (2) Else -> find somewhere enough else, copy the data from mem_address,
    *     return the new address.
    * Examples:
    * - if(mem_address == NULL && size != 0) ==> malloc(newsize)
    * - if(mem_address != NULL && size == 0) ==> free(mem_address)
    * - if(mem_address != NULL && size != 0) ==> free(mem_address)+malloc(newsize)
    */
    return realloc(ptr, newSize);
}

// find the nearest 2-power which is >= v
uint32_t ceilToPowerOf2(uint32_t v){
    v += (v==0); // handle the '0'
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

DECLARE_BUFFER_METHOD(String)
DECLARE_BUFFER_METHOD(Byte)
DECLARE_BUFFER_METHOD(Int)
DECLARE_BUFFER_METHOD(Char)

void symbolTableClear(VM* vm, SymbolTable* buffer){
    uint32_t index = 0;
    while(index < buffer->count){
        memManager(vm, buffer->datas[index++].str, 0, 0);
    }
    StringBufferClear(vm, buffer);
}

void errorReport(void* parser, ErrorType errorType, const char* fmt, ...) {
    char buffer[DEFAULT_BUfFER_SIZE] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buffer, DEFAULT_BUfFER_SIZE, fmt, ap);
    va_end(ap);

    switch (errorType) {
        case ERROR_IO:
        case ERROR_MEM:
            fprintf(stderr, "%s:%d In function %s():%s\n",
                __FILE__, __LINE__, __func__, buffer);
            break;
        case ERROR_LEX:
        case ERROR_COMPILE:
            ASSERT(parser != NULL, "parser is null!");
            fprintf(stderr, "%s:%d \"%s\"\n", ((Parser*)parser)->file,
                ((Parser*)parser)->preToken.lineNo, buffer);
            break;
        case ERROR_RUNTIME:
            fprintf(stderr, "%s\n", buffer);
            break;
        default:
            NOT_REACHED();
    }

   exit(1);
}
