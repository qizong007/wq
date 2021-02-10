#include "obj_map.h"
#include "class.h"
#include "vm.h"
#include "obj_string.h"
#include "obj_range.h"

ObjMap* newObjMap(VM* vm) {
    ObjMap* objMap = ALLOCATE(vm, ObjMap);
    initObjHeader(vm, &objMap->objHeader, OT_MAP, vm->mapClass);
    objMap->capacity = objMap->count = 0;
    objMap->entries = NULL;
    return objMap;
}

// calculate the number's hashCode
static uint32_t hashNum(double num) {
    Bits64 bits64;
    bits64.num = num;
    return bits64.bits32[0] ^ bits64.bits32[1];
}

// calculate the object's hashCode
static uint32_t hashObj(ObjHeader* objHeader) {
    switch (objHeader->type) {
        case OT_CLASS:  
            return hashString(((Class*)objHeader)->name->value.start,
                ((Class*)objHeader)->name->value.length);
        case OT_RANGE: { 
            ObjRange* objRange = (ObjRange*)objHeader;
            return hashNum(objRange->from) ^ hashNum(objRange->to);
        }
        case OT_STRING:  
            return ((ObjString*)objHeader)->hashCode;
        default:
            RUN_ERROR("the hashable are objstring, objrange and class.");
    }
    return 0;
}

// value -> hash
static uint32_t hashValue(Value value) {
    switch (value.type) {
        case VT_FALSE:
            return 0;
        case VT_NULL:
            return 1;
        case VT_NUM:
            return hashNum(value.num);
        case VT_TRUE:
            return 2;
        case VT_OBJ:
            return hashObj(value.objHeader);
        default:
            RUN_ERROR("unsupport type hashed!");
    }
    return 0;
}

static bool addEntry(Entry* entries, uint32_t capacity, Value key, Value value) {
    uint32_t index = hashValue(key) % capacity;

    // open-detection to find slot
    while (true) {
        if (entries[index].key.type == VT_UNDEFINED) {
            entries[index].key = key;
            entries[index].value = value;
            return true;	   
        } else if (valueIsEqual(entries[index].key, key)) { 
            entries[index].value = value;
            return false;	
        }
        // next slot
        index = (index + 1) % capacity;
    }
}

static void resizeMap(VM* vm, ObjMap* objMap, uint32_t newCapacity) {
    // 1 build a new entry array
    Entry* newEntries = ALLOCATE_ARRAY(vm, Entry, newCapacity);
    uint32_t idx = 0;
    while (idx < newCapacity) {
        newEntries[idx].key = VT_TO_VALUE(VT_UNDEFINED);
        newEntries[idx].value = VT_TO_VALUE(VT_FALSE);
        idx++;
    }
    // 2 reverse old array, insert values
    if (objMap->capacity > 0) {
        Entry* entryArr = objMap->entries;
        idx = 0;
        while (idx < objMap->capacity) {
            // this slot has value
            if (entryArr[idx].key.type != VT_UNDEFINED) {
                addEntry(newEntries, newCapacity, entryArr[idx].key, entryArr[idx].value);
            }
            idx++;
        }
    }
    // 3 free old space
    DEALLOCATE_ARRAY(vm, objMap->entries, objMap->count);
    objMap->entries = newEntries;    
    objMap->capacity = newCapacity;    
}

static Entry* findEntry(ObjMap* objMap, Value key) {
    if (objMap->capacity == 0) {
        return NULL;
    }
    uint32_t index = hashValue(key) % objMap->capacity;
    Entry* entry; 
    while (true) {
        entry = &objMap->entries[index];
        // open-detection 
        if (valueIsEqual(entry->key, key)) {
            return entry;
        }
        if (VALUE_IS_UNDEFINED(entry->key) && VALUE_IS_FALSE(entry->value)) {
            return NULL;    
        }
        index = (index + 1) % objMap->capacity;
    }
}

void mapSet(VM* vm, ObjMap* objMap, Value key, Value value) {
    // when used hit 80%, we should resize the map
    if (objMap->count + 1 > objMap->capacity * MAP_LOAD_FACTOR) {
        uint32_t newCapacity = objMap->capacity * CAPACITY_GROW_FACTOR;
        if (newCapacity < MIN_CAPACITY) {
            newCapacity = MIN_CAPACITY;
        }
        resizeMap(vm, objMap, newCapacity);
    }
    // if add new key, then objMap->count++
    if (addEntry(objMap->entries, objMap->capacity, key, value)) {
        objMap->count++;
    }
}

Value mapGet(ObjMap* objMap, Value key) {
    Entry* entry = findEntry(objMap, key);
    if (entry == NULL) {
        return VT_TO_VALUE(VT_UNDEFINED);
    }
    return entry->value;
}

void clearMap(VM* vm, ObjMap* objMap) {
    DEALLOCATE_ARRAY(vm, objMap->entries, objMap->count);
    objMap->entries = NULL;
    objMap->capacity = objMap->count = 0;
}

Value removeKey(VM* vm, ObjMap* objMap, Value key) {
    Entry* entry = findEntry(objMap, key);
    if (entry == NULL) {
        return VT_TO_VALUE(VT_NULL);
    }
    Value value = entry->value;
    entry->key = VT_TO_VALUE(VT_UNDEFINED); 
    entry->value = VT_TO_VALUE(VT_TRUE);   
    objMap->count--;  
    if (objMap->count == 0) { 
        clearMap(vm, objMap);
    } else if (objMap->count < objMap->capacity / (CAPACITY_GROW_FACTOR) * MAP_LOAD_FACTOR &&
               objMap->count > MIN_CAPACITY) {   
        uint32_t newCapacity = objMap->capacity / CAPACITY_GROW_FACTOR;
        if (newCapacity < MIN_CAPACITY) {
            newCapacity = MIN_CAPACITY;
        }
        resizeMap(vm, objMap, newCapacity);
    }
    return value;
}