#ifndef LUMEN_TOKEN_H
#define LUMEN_TOKEN_H

#include <stddef.h>

#define TOKEN_DEFAULT_COUNT 256
#define TOKEN_BUFFER_SIZE 32

typedef enum {
    TOK_EQUALS,
    TOK_ADD,
    TOK_SUBTRACT,
    TOK_MULTIPLY,
    TOK_DIVIDE,
    TOK_PAREN_OPEN,
    TOK_PAREN_CLOSE,
    TOK_BRACE_OPEN,
    TOK_BRACE_CLOSE,
    TOK_BRACKET_OPEN,
    TOK_BRACKET_CLOSE,
    TOK_COMMA,
    TOK_ADDRESS,
    TOK_POINTER,
    TOK_SEMICOLON,
    TOK_RETURN,
    TOK_EXIT,
    TOK_STRUCT,
    TOK_TYPE,

    TOK_VARIABLE,
    TOK_FUNC_NAME,
    TOK_INT_LITERAL,
    TOK_STRING_LITERAL,
} TokenType;

typedef struct {
    TokenType type;
    int has_value;  // boolean
    char value[TOKEN_BUFFER_SIZE];
    int line;  // line number where token is defined in source
} Token;

typedef struct {
    Token* tokens;
    size_t capacity;
    size_t count;
} TokenArray;

void token_init(const char* str, Token* token, TokenArray* tokens);
void token_print(const Token* token);
void tokens_from_source(const char* src, TokenArray* tokens);

#endif  /* LUMEN_TOKEN_H */
