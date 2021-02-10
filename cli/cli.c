#include <stdio.h>
#include <string.h>
#include "cli.h"
#include "parser.h"
#include "vm.h"
#include "core.h"

static void runFile(const char* path) {
    const char* lastSlash = strrchr(path, '/');
    // the script file is not under the current working directory
    if (lastSlash != NULL) {
        char* root = (char*)malloc(lastSlash - path + 2);
        memcpy(root, path, lastSlash - path + 1);
        root[lastSlash - path + 1] = '\0';
        rootDir = root;
    }

    VM* vm = newVM();
    const char* sourceCode = readFile(path);
    
    Parser parser;
    initParser(vm, &parser, path, sourceCode, NULL);

    #include "token.list"

    while (parser.curToken.type != TOKEN_EOF) {
        getNextToken(&parser);
        printf("%dL-tokenArray[%d]: %s [", parser.curToken.lineNo, \
            parser.curToken.type,tokenArray[parser.curToken.type]);
        uint32_t idx = 0;
        while (idx < parser.curToken.length) {
            printf("%c", *(parser.curToken.start+idx++));
        }
        printf("]\n");
    }
}

int main(int argc, const char** argv) {
    if (argc == 1) {
        ;
    } else {
        runFile(argv[1]);
    }
    return 0;
}
