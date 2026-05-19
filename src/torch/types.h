#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TYPE_NAME_SIZE 32
#define TYPES_DEFAULT_CAPACITY 32

typedef struct {
    char name[TYPE_NAME_SIZE];
    unsigned int id;  // unique id for each type
} TypeObj;

typedef struct {
    TypeObj* types;
    size_t count;
    size_t capacity;

    int _struct_start;
} Types;

Types types;

void types_append_primitives();

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

void types_append(const char* name) {
    if (types.count >= types.capacity) {
        types.capacity *= 2;
        types.types = realloc(types.types, types.capacity);
        if (!types.types) {
            printf("types realloc failed\n");
        }
    }

    TypeObj _type;
    memset(_type.name, 0, TYPE_NAME_SIZE);
    strncpy(_type.name, name, TYPE_NAME_SIZE);
    _type.id = types.count;
    types.types[types.count++] = _type;

    printf("TYPES NEW TYPE ADDED: '%s'\n", _type.name);
}

void types_append_primitives() {
    types_append("NULL");
    types_append("i8");
    types_append("i16");
    types_append("i32");
    types_append("i64");
    types_append("ui8");
    types_append("ui16");
    types_append("ui32");
    types_append("ui64");

    types._struct_start = (int)types.count;
}
