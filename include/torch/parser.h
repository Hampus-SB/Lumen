#ifndef LUMEN_PARSER_H
#define LUMEN_PARSER_H

#include "tokenizer.h"
#include "types.h"

#define NODE_CHILDREN_COUNT 8
#define NODE_ROOT_CHILDREN_COUNT 512

#define BUILTIN_ASSEMBLY_SYMBOL "asm"

typedef enum {
    NODE_IGNORE,  // dont remember what this is for
    NODE_DECLARATION,  // variable declaration
    NODE_STATEMENT,
    NODE_EXPRESSION,
    NODE_OPERATOR,
    NODE_DECLARATION_FUNC,
    NODE_CALL_FUNC,
    NODE_STRUCT,
    NODE_LOCAL_VARIABLE,
    NODE_ASSEMBLY,
    NODE_INDEX,  // might remove or something
    NODE_RETURN,
    NODE_IF,
    NODE_BOOLEAN_EXPRESSION,
} NodeType;

typedef struct Node {
    NodeType type;
    struct Node* parent;
    struct Node* children;
    int len;  // how many children

    Token* token;
    TypeObj* type_info;
} Node;

void print_tree(const Node* root);
void parse(Node* root, TokenArray tokens);

#endif  /* LUMEN_PARSER_H */
