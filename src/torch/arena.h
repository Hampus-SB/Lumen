#ifndef LUMEN_ARENA_H
#define LUMEN_ARENA_H

#define ARENA_DEFAULT_SIZE 524288

typedef struct {
    void* memory;
    void* pos;
    size_t size;
    size_t prev_alloc_size;
} Arena;

void arena_init(Arena* arena, size_t size);
void arena_free(Arena* arena);
void* arena_alloc(Arena* arena, size_t size);
void arena_extend(Arena* arena);

#endif  /* LUMEN_ARENA_H */



#ifdef LUMEN_ARENA_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>

void arena_init(Arena* arena, size_t size) {
    arena->memory = malloc(size);
    arena->pos = arena->memory;
    arena->size = size;
    arena->prev_alloc_size = 0;
}

void arena_free(Arena* arena) {
    free(arena->memory);
}

void* arena_alloc(Arena* arena, size_t size) {
    if (arena->pos + size > arena->memory + arena->size) {
        fprintf(stderr, "Arena full.\n");
        return NULL;
    }
    arena->pos += size;
    arena->prev_alloc_size = size;
    return arena->pos - size;
}

// extends last allocated chunk
void arena_extend(Arena* arena) {
    if (arena->pos + arena->prev_alloc_size > arena->memory + arena->size) {
        fprintf(stderr, "Cannot extend arena.\n");
        return;
    }
    arena->pos += arena->prev_alloc_size;
    arena->prev_alloc_size *= 2;
}

Arena arena;

#endif  /* LUMEN_ARENA_IMPLEMENTATION */
