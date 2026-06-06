#include "../../include/torch/tests.h"
#include "../../include/torch/arena.h"
#include "../../include/torch/symbols.h"
#include "../../include/torch/type_checker.h"
#include "../../include/torch/generation.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "logging.h"

void test_run_path(const char* path, const char* file_name) {
    arena_init(get_arena(), ARENA_DEFAULT_SIZE);
    types_init();
    symbol_table_init();

    FILE* fh = fopen(path, "r");
    if (fh == NULL) {
        logerror("Failed to open source file. '%s'", path);
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

    print_tree(&root);

    if (!validate_types(&root)) {
        logerror("Failed to validate types.");
    }

    generate_asm(&root, file_name);

    arena_free(get_arena());
    types_free();
    symbol_table_free();
}

void test_compile(const char* file_name) {
    char command[128] = {0};

    sprintf(command, "nasm -f elf64 tests/output/%s.asm -o tests/output/%s.o"
        , file_name, file_name);
    system(command);

    sprintf(command, "ld tests/output/%s.o -o tests/output/%s.out"
        , file_name, file_name);
    system(command);

    sprintf(command, "rm tests/output/%s.o", file_name);
    system(command);
}

void test_pointers() {
    printf(" --- TEST: started 'pointers' --- \n");
    test_run_path("tests/src/pointers.lu", "tests/output/pointers.asm");
    test_compile("pointers");
    printf(" --- TEST: finsished 'pointers' --- \n\n");
}

void test_functions() {
    printf(" --- TEST: started 'functions' --- \n");
    test_run_path("tests/src/functions.lu", "tests/output/functions.asm");
    test_compile("functions");
    printf(" --- TEST: finsished 'functions' --- \n\n");
}

void test_structs() {
    printf(" --- TEST: started 'structs' --- \n");
    test_run_path("tests/src/structs.lu", "tests/output/structs.asm");
    test_compile("structs");
    printf(" --- TEST: finsished 'structs' --- \n\n");
}

void test_comments() {
    printf(" --- TEST: started 'comments' --- \n");
    test_run_path("tests/src/comments.lu", "tests/output/comments.asm");
    test_compile("comments");
    printf(" --- TEST: finsished 'comments' --- \n\n");
}

void test_programs() {
    printf("TEST: starting tests\n\n");

    test_pointers();
    test_functions();
    test_comments();
    test_structs();

    printf("TEST: finished tests\n\n");
}
