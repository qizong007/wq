#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "utils.h"
#include "utf8.h"
#include <string.h>
#include <ctype.h>

typedef struct {
   char* keyword;
   uint8_t length; // eg. length('var') is 3
   TokenType token;
} KeywordToken; 

// keyword table
KeywordToken keywordTable[] = {
    {"var",	      3,	TOKEN_VAR}, 
    {"fun",	      3,	TOKEN_FUN}, 
    {"if",	      2,	TOKEN_IF}, 
    {"else",	  4,  	TOKEN_ELSE},
    {"true",	  4,  	TOKEN_TRUE}, 
    {"false",	  5,  	TOKEN_FALSE}, 
    {"while",	  5,  	TOKEN_WHILE}, 
    {"for",	      3,  	TOKEN_FOR}, 
    {"break",	  5,  	TOKEN_BREAK}, 
    {"continue",  8,    TOKEN_CONTINUE},
    {"return",	  6,  	TOKEN_RETURN}, 
    {"null",	  4,  	TOKEN_NULL}, 
    {"class",	  5,  	TOKEN_CLASS},
    {"is",	      2,  	TOKEN_IS},
    {"static",	  6,  	TOKEN_STATIC},
    {"this",	  4,  	TOKEN_THIS},
    {"super",	  5,  	TOKEN_SUPER},
    {"import",	  6,  	TOKEN_IMPORT},
    {NULL,	      0,  	TOKEN_UNKNOWN}
};

static TokenType isKeyword(const char* start, uint32_t length) {
    uint32_t index = 0;
    while (keywordTable[index].keyword != NULL) {
        if (keywordTable[index].length == length && \
            memcmp(keywordTable[index].keyword, start, length) == 0) {
            // is keyword, return the token(key word)
            return keywordTable[index].token;
        }
        index++;
    }
    // is not keyword, return the token(variable) 
    return TOKEN_ID;
}

char lookAheadChar(Parser* parser) {
    return *parser->nextCharPtr;
}

// read in and next
static void getNextChar(Parser* parser) {
    parser->curChar = *parser->nextCharPtr++;
}

// is the next char what we expected?
static bool matchNextChar(Parser* parser, char expectedChar) {
    if (lookAheadChar(parser) == expectedChar) {
        // read in the next character
        getNextChar(parser);
        return true;
    }
    return false;
}

static void skipBlanks(Parser* parser) {
    while (isspace(parser->curChar)) {
        if (parser->curChar == '\n') {
            // return the next line 
            parser->curToken.lineNo++;
        }
        getNextChar(parser);
    }
}

