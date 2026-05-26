#ifndef LUMEN_TYPES_H
#define LUMEN_TYPES_H

#include <stddef.h>

#define TYPE_NAME_SIZE 32
#define TYPES_DEFAULT_CAPACITY 32

typedef enum {
    BITS_NONE,
    BITS_8,
    BITS_16,
    BITS_32,
    BITS_64,
} RegisterSize;

typedef struct {
    char name[TYPE_NAME_SIZE];
    unsigned int id;  // unique id for each type
    RegisterSize size;
} TypeObj;

typedef struct {
    TypeObj* types;
    size_t count;
    size_t capacity;

    int _struct_start;
} Types;

void types_init();
void types_free();
void types_append(const char* name, RegisterSize size);

TypeObj* types_get_type_obj(const char* name);

int types_is_struct(const TypeObj* type);
int types_exists(const char* name);
int types_is_ptr(const TypeObj* type);

const char* register_from_type_obj(const TypeObj* type, int count);

#endif  /* LUMEN_TYPES_H */
