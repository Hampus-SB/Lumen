#ifndef LEXER_H
#define LEXER_H

// Maybe 16 idk
#define MAX_TOKENS 16

// Token types
typedef enum {
    TOK_LABEL,
    TOK_IDENT,
    TOK_REGISTER,
    TOK_NUMBER,
    TOK_STRING,
    TOK_MEMMAN,
    TOK_COMMA
} TokenType;

// Sections
typedef enum {
    SEC_NONE,
    SEC_TEXT,
    SEC_DATA
} Section;

// Token structure
typedef struct {
    TokenType type;
    char text[64];
} Token;

int lex_line(const char *line, Token tokens[], int max_tokens);

#endif
