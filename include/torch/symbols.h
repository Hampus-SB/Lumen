#ifndef LUMEN_SYMBOLS_H
#define LUMEN_SYMBOLS_H

#include "parser.h"

#include <stddef.h>

#define DEFAULT_SYMBOL_COUNT 32

typedef enum {
    SYM_VARIABLE,
    SYM_FUNCTION,
    SYM_STRUCT,
} SymbolType;

typedef struct {
    SymbolType type;
    char* name;
    Node* node;
} Symbol;

typedef struct {
    Symbol* symbols;
    int capacity;
    int len;
} SymbolTable;

void symbol_table_init();
void symbol_table_append(Node* node);

// get node of func decl or struct decl
Node* symbol_table_find_struct(const TypeObj* type);

// get type of known variable or function
TypeObj* symbol_table_find_type(const char* name);

#endif  /* LUMEN_SYMBOLS_H */
