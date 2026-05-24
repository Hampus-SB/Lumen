#include "../../include/torch/types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Types types;

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
