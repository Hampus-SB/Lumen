#ifndef LUMEN_GENERATION_H
#define LUMEN_GENERATION_H

#include "parser.h"

#define STACK_SIZE 64
#define VARIABLE_NAME_SIZE 32
#define MAX_FUNCTIONS 1024

typedef struct {
    char name[VARIABLE_NAME_SIZE];
    int offset;  // offset from rbp
    int size;  // size on stack in bytes
    TypeObj* type;
} Variable;

typedef struct {
    Variable variables[STACK_SIZE];
    int index;

    // how many variables in i stack
    int count[MAX_FUNCTIONS];
    int _index;  // which 'stack'
} Stack;

void generate_asm(const Node* root, const char* out_path);

#endif  /* LUMEN_GENERATION_H */
