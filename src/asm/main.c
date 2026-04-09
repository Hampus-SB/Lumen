#include <stdio.h>
#include "lexer.h"

#define SIZE_OF_BUFF 128

int main(int argc, char** argv) {

    // check if there is an input file
    if (argc == 1) {
        printf("Must input a file\n");
        return 1;
    } 
  
    FILE* inFile = fopen(argv[1], "r");
    if (!inFile) {
        printf("Error loading file\n");
        return 1;
    }

    char buff[SIZE_OF_BUFF];
    Token tokens[MAX_TOKENS];

    while (fgets(buff, SIZE_OF_BUFF, inFile) != NULL) {
        
        int count = lex_line(buff, tokens, MAX_TOKENS);

        // print tokens (debug, remove later)
        for (int i = 0; i < count; i++) {
            printf("Type: %d, Text: %s\n", tokens[i].type, tokens[i].text);
        }

        printf("\n");
    }

    fclose(inFile);
    return 0;
}
