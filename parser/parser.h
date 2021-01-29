#ifndef _PARSER_PARSER_H
#define _PARSER_PARSER_H
#include "common.h"
#include "vm.h"

typedef enum {
    TOKEN_UNKNOWN,
    // data type
    TOKEN_NUM,		        // number
    TOKEN_STRING,     	    // string
    TOKEN_ID,	     	    // variable
    TOKEN_INTERPOLATION,    // internal expression

    // key word(save for system)
    TOKEN_VAR,		        //'var'
    TOKEN_FUN,		        //'fun'
    TOKEN_IF,		        //'if'
    TOKEN_ELSE,	     	    //'else'	
    TOKEN_TRUE,	     	    //'true'
    TOKEN_FALSE,	     	//'false'
    TOKEN_WHILE,	     	//'while'
    TOKEN_FOR,	     	    //'for'
    TOKEN_BREAK,	     	//'break'
    TOKEN_CONTINUE,         //'continue'
    TOKEN_RETURN,     	    //'return'
    TOKEN_NULL,	     	    //'null'

    // class and module
    TOKEN_CLASS,	     	//'class'
    TOKEN_THIS,	     	    //'this'
    TOKEN_STATIC,     	    //'static'
    TOKEN_IS,		        // 'is'
    TOKEN_SUPER,	     	//'super'
    TOKEN_IMPORT,     	    //'import'

    // delimiter
    TOKEN_COMMA,		    //','
    TOKEN_COLON,		    //':'
    TOKEN_LEFT_PAREN,	    //'('
    TOKEN_RIGHT_PAREN,	    //')'
    TOKEN_LEFT_BRACKET,	    //'['
    TOKEN_RIGHT_BRACKET,	//']'
    TOKEN_LEFT_BRACE,	    //'{'
    TOKEN_RIGHT_BRACE,	    //'}'
    TOKEN_DOT,		        //'.'
    TOKEN_DOT_DOT,	        //'..'

    // simple binocular operator
    TOKEN_ADD,		        //'+'
    TOKEN_SUB,		        //'-'
    TOKEN_MUL,		        //'*' 
    TOKEN_DIV,		        //'/' 
    TOKEN_MOD,		        //'%'

    // assign operator
    TOKEN_ASSIGN,	        //'='

    // bit operator
    TOKEN_BIT_AND,	        //'&'
    TOKEN_BIT_OR,	        //'|'
    TOKEN_BIT_NOT,	        //'~'
    TOKEN_BIT_SHIFT_RIGHT,  //'>>'
    TOKEN_BIT_SHIFT_LEFT,   //'<<'

    // logic operator
    TOKEN_LOGIC_AND,	    //'&&'
    TOKEN_LOGIC_OR,	        //'||'
    TOKEN_LOGIC_NOT,	    //'!'

    // relation operator
    TOKEN_EQUAL,		    //'=='
    TOKEN_NOT_EQUAL,	    //'!='
    TOKEN_GREATE,	        //'>'
    TOKEN_GREATE_EQUAL,	    //'>='
    TOKEN_LESS,		        //'<'
    TOKEN_LESS_EQUAL,	    //'<='

    TOKEN_QUESTION,	        //'?'

    // file ending mark
    TOKEN_EOF		        //'EOF'
} TokenType;

/* lex(in) -> parser(process) -> token(out) */
typedef struct {
    TokenType type;
    const char* start;
    uint32_t length;
    uint32_t lineNo;
} Token;

struct parser {
    const char* file;       // file name
    const char* sourceCode; // buffer area
    const char* nextCharPtr;
    char curChar;
    Token curToken;
    Token preToken;
    // record the num of %()
    int interpolationExpectRightParenNum;
    VM* vm;
} ;

#define PEEK_TOKEN(parserPtr) parserPtr->curToken.type

char lookAheadChar(Parser* parser);
void getNextToken(Parser* parser);
bool matchToken(Parser* parser, TokenType expected);
void consumeCurToken(Parser* parser, TokenType expected, const char* errMsg);
void consumeNextToken(Parser* parser, TokenType expected, const char* errMsg);
uint32_t getByteNumOfEncodeUtf8(int value);
uint8_t encodeUtf8(uint8_t* buf, int value);
void initParser(VM* vm, Parser* parser, const char* file, const char* sourceCode);

#endif