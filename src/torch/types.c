#include "../../include/torch/types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Types types;

const char* registers[] = {
    "rax", "rbx", "rcx", "rdx",
    "eax", "ebx", "ecx", "edx",
    "ax", "bx", "cx", "dx",
    "al", "bl", "cl", "dl",
};

void types_append_primitives() {
    types_append("NULL", BITS_NONE);
    types_append("i8", BITS_8);
    types_append("i16", BITS_16);
    types_append("i32", BITS_32);
    types_append("i64", BITS_64);
    types_append("ui8", BITS_8);
    types_append("ui16", BITS_16);
    types_append("ui32", BITS_32);
    types_append("ui64", BITS_64);

    types._struct_start = (int)types.count;
}

void types_init() {
    types.count = 0;
    types.capacity = TYPES_DEFAULT_CAPACITY;
    types.types = malloc(sizeof(TypeObj) * types.capacity);

    types_append_primitives();
}

void types_free() {
    if (types.types) {
        free(types.types);
    }
}

void types_append(const char* name, RegisterSize size) {
    if (types.count >= types.capacity) {
        types.capacity *= 2;
        types.types = realloc(types.types, types.capacity);
        if (!types.types) {
            printf("types realloc failed\n");
        }
    }

    TypeObj type;
    memset(type.name, 0, TYPE_NAME_SIZE);
    strncpy(type.name, name, TYPE_NAME_SIZE);
    type.id = types.count;
    type.size = size;
    types.types[types.count++] = type;

    TypeObj type_ptr;
    memset(type_ptr.name, 0, TYPE_NAME_SIZE);
    strncpy(type_ptr.name, name, TYPE_NAME_SIZE);
    strcat(type_ptr.name, "&");
    type_ptr.id = types.count;
    type_ptr.size = BITS_64;
    types.types[types.count++] = type_ptr;

    printf("NEW TYPE ADDED: '%s'\n", type.name);
    printf("NEW TYPE ADDED: '%s'\n", type_ptr.name);
}

TypeObj* types_get_type_obj(const char* name) {
    for (int i = 0; i < types.count; i++) {
        if (strcmp(name, types.types[i].name) == 0) {
            return &types.types[i];
        }
    }
    fprintf(stderr, "No type with the name '%s'.\n", name);
    return NULL;
}

int types_is_struct(const TypeObj* type) {
    if (type->id >= types._struct_start)
        return 1;
    return 0;
}

int types_exists(const char* name) {
    for (int i = 0; i < types.count; i++) {
        if (strcmp(name, types.types[i].name) == 0) {
            return 1;
        }
    }
    return 0;
}

int types_is_ptr(const TypeObj* type) {
    return type->name[strlen(type->name) - 1] == '&';
}

const char* register_from_type_obj(const TypeObj* type, int count) {
    switch (type->size) {
        case BITS_64:
            return registers[0 + count - 1];
        case BITS_32:
            return registers[4 + count - 1];
        case BITS_16:
            return registers[8 + count - 1];
        case BITS_8:
            return registers[12 + count - 1];
        default:
            return "NO REGISTER FOUND";
    }
}
