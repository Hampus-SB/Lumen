#include "../../include/torch/symbols.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SymbolTable symbol_table;

void symbol_table_init() {
    symbol_table.symbols = malloc(sizeof(Symbol) * DEFAULT_SYMBOL_COUNT);
    symbol_table.capacity = DEFAULT_SYMBOL_COUNT;
    symbol_table.len = 0;
}

void symbol_table_free() {
    if (symbol_table.symbols) {
        free(symbol_table.symbols);
    }
    symbol_table.capacity = DEFAULT_SYMBOL_COUNT;
    symbol_table.len = 0;
}

void symbol_table_append(Node* node) {
    if (symbol_table.len >= symbol_table.capacity) {
        symbol_table.capacity *= 2;
        symbol_table.symbols = realloc(symbol_table.symbols, symbol_table.capacity);
        printf("Extended symbol table\n");
    }

    Symbol* symbol = &symbol_table.symbols[symbol_table.len++];
    symbol->node = node;
    symbol->name = node->token->value;

    if (node->type == NODE_DECLARATION)
        symbol->type = SYM_VARIABLE;
    else if (node->type == NODE_LOCAL_VARIABLE)
        symbol->type = SYM_VARIABLE;
    else if (node->type == NODE_IGNORE)
        symbol->type = SYM_VARIABLE;
    else if (node->type == NODE_DECLARATION_FUNC)
        symbol->type = SYM_FUNCTION;
    else if (node->type == NODE_STRUCT)
        symbol->type = SYM_STRUCT;
    else {
        printf("Invalid node to create symbol. :%i\n", node->token->line);
    }

    //printf("Added symbol '%s', '%s' | %p\n", symbol->name,
    //    symbol->node->type_info->name, symbol->node);
}

Node* symbol_table_find_struct(const TypeObj* type) {
    for (int i = 0; i < symbol_table.len; i++) {
        if (strcmp(type->name, symbol_table.symbols[i].name) == 0) {
            return symbol_table.symbols[i].node;
        }
    }
    fprintf(stderr, "No struct with the name '%s'.\n", type->name);
    return NULL;
}

TypeObj* symbol_table_find_type(const char* name) {
    for (int i = 0; i < symbol_table.len; i++) {
        if (strcmp(name, symbol_table.symbols[i].name) == 0) {
            return symbol_table.symbols[i].node->type_info;
        }
    }
    fprintf(stderr, "No symbol with the name '%s'.\n", name);
    return NULL;
}
