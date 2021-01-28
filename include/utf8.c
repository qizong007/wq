#include "utf8.h"
#include "common.h"

uint32_t getByteNumOfEncodeUtf8(int value){
    ASSERT(value > 0, "Can`t encode negative value!");
    if(value <= 0x7f) return 1;
    if(value <= 0x7ff) return 2;
    if(value <= 0xffff) return 3;
    if(value <= 0x10ffff) return 4;
    return 0; 
}

uint32_t getByteNumOfDecodeUtf8(uint8_t byte){
    if ((byte & 0xc0) == 0x80) return 0; // 10 xxxxxx(low)
    if ((byte & 0xf8) == 0xf0) return 4; // 11110 xxx(high)
    if ((byte & 0xf0) == 0xe0) return 3; // 1110 xxxx(high)
    if ((byte & 0xe0) == 0xc0) return 2; // 110 xxxxx(high)
    return 1; // ascii: 0 xxxxxxx
}

// encode value and put it into buf, return the byte num we wrote in
uint8_t encodeUtf8(uint8_t* buf, int value){
    ASSERT(value > 0, "Can`t encode negative value!");
    // ***big-endian***
    if(value <= 0x7f) { // ascii: 0 xxxxxxx
        *buf = value & 0x7f;
        return 1;
    } else if(value <= 0x7ff) { // 110 xxxxx(high)
        *buf++ = 0xc0 | ((value & 0x7c0) >> 6);
        *buf = 0x80 | (value & 0x3f);
        return 2;
    } else if(value <= 0xffff) { // 1110 xxxx(high)
        *buf++ = 0xe0 | ((value & 0xf000) >> 12);
        *buf++ = 0x80 | ((value & 0xfc0) >> 6);
        *buf = 0x80 | (value & 0x3f);
        return 3;
    } else if(value <= 0x10ffff) { // 11110 xxx(high)
        *buf++ = 0xf0 | ((value & 0x1c0000) >> 18);
        *buf++ = 0x80 | ((value & 0x3f000) >> 12);
        *buf++ = 0x80 | ((value & 0xfc0) >> 6);
        *buf = 0x80 | (value & 0x3f);
        return 4;
    }
    NOT_REACHED();
    return 0;
}

int decodeUtf8(const uint8_t* bytePtr, uint32_t length){
    // ascii: 0 xxxxxxx
    if(*bytePtr <= 0x7f) return *bytePtr;

    int value;
    uint32_t remainingBytes;

    // handle the first byte
    if((*bytePtr && 0xe0) == 0xc0) { // 110 xxxxx(high)
        value = *bytePtr & 0x1f;
        remainingBytes = 1;
    } else if((*bytePtr && 0xf0) == 0xe0) { // 1110 xxxx(high)
        value = *bytePtr & 0x0f;
        remainingBytes = 2;
    } else if((*bytePtr && 0x80) == 0xf0) { // 11110 xxx(high)
        value = *bytePtr & 0x07;
        remainingBytes = 3;
    } else {
        return -1;
    }

    if(remainingBytes > length - 1) return -1;

    // handle the remaining bytes
    while(remainingBytes > 0) {
        bytePtr++;
        // it should be like: 10 xxxxxx(low)
        if((*bytePtr & 0xc0) != 0x80){
            return -1;
        }
        value = value << 6 | (*bytePtr & 0x3f);
        remainingBytes--;
    }
    return value;
} 