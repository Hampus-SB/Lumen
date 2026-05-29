#include "../../include/torch/tests.h"
#include "../../include/torch/arena.h"
#include "../../include/torch/symbols.h"
#include "../../include/torch/type_checker.h"
#include "../../include/torch/generation.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void test_run_path(const char* path, const char* file_name) {
    arena_init(get_arena(), ARENA_DEFAULT_SIZE);
    types_init();
    symbol_table_init();

    FILE* fh = fopen(path, "r");
    if (fh == NULL) {
        printf("ERROR: Failed to open source file. '%s'\n", path);
        exit(EXIT_FAILURE);
    }

    char file[8192] = {0};

    char buffer[8192];
    while (fgets(buffer, 8192, fh) != NULL) {
        strcat(file, buffer);
    }

    TokenArray tokens;
    tokens.capacity = TOKEN_DEFAULT_COUNT;
    tokens.count = 0;
    tokens.tokens = arena_alloc(get_arena(), tokens.capacity * sizeof(Token));

    tokens_from_source(file, &tokens);
    for (int i = 0; i < tokens.count; i++) {
        //token_print(&tokens.tokens[i]);
    }

    Node root;
    root.children = arena_alloc(get_arena(), sizeof(Node) * NODE_ROOT_CHILDREN_COUNT);
    root.len = 0;

    parse(&root, tokens);

    if (!validate_types(&root)) {
        printf("ERROR: Failed to validate types.\n");
    }

    generate_asm(&root, file_name);

    arena_free(get_arena());
    types_free();
    symbol_table_free();
}

void test_pointers() {
    printf("TEST: started 'pointers'\n");

    test_run_path("tests/src/pointers.lu", "tests/output/pointers.asm");

    printf("TEST: finsished 'pointers'\n");
}

void test_functions() {
    printf("TEST: started 'functions'\n");

    test_run_path("tests/src/functions.lu", "tests/output/functions.asm");

    printf("TEST: finsished 'functions'\n");
}

void test_arrays() {}

void test_programs() {
    printf("TEST: starting tests\n");

    test_pointers();
    test_functions();
    test_arrays();

    printf("TEST: finished tests\n\n\n\n");
}
