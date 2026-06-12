#include "../../include/torch/arena.h"
#include "../../include/torch/types.h"
#include "../../include/torch/tokenizer.h"
#include "../../include/torch/parser.h"
#include "../../include/torch/symbols.h"
#include "../../include/torch/type_checker.h"
#include "../../include/torch/generation.h"
#include "../../include/torch/tests.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_BUFFER_SIZE 8192
#define FILE_BUFFER_LINE_SIZE 128

int main(int argc, char** argv) {
	test_programs();
	return 0;

	/*
	if (argc != 2) {
		printf("No source file given.\n");
		exit(EXIT_FAILURE);
	}

	FILE* fh = fopen(argv[1], "r");
	if (fh == NULL) {
		printf("Failed to open source file.\n");	
		exit(EXIT_FAILURE);
	}

	char file[FILE_BUFFER_SIZE] = {0};

	char buffer[FILE_BUFFER_LINE_SIZE];
	while (fgets(buffer, FILE_BUFFER_LINE_SIZE, fh) != NULL) {
		strcat(file, buffer);
	}

	fclose(fh);

	arena_init(get_arena(), ARENA_DEFAULT_SIZE);
	types_init();
	symbol_table_init();

    	char file2[8192] = {0};

    	char buffer[8192];
    	while (fgets(buffer, 8192, fh) != NULL) {
        	strcat(file, buffer);
    	}

    	int out_len = 0;
    	comments_remove(file, 8192, file2, &out_len);
    	memset(file, 0, 8192);

    	import(file2, 8192, file, &out_len);
	TokenArray tokens;
	tokens.capacity = TOKEN_DEFAULT_COUNT;
	tokens.count = 0;
	tokens.tokens = arena_alloc(get_arena(), tokens.capacity * sizeof(Token));

	tokens_from_source(file, &tokens);
	for (int i = 0; i < tokens.count; i++) {
		//token_print(&tokens.tokens[i]);
	}
	printf("generated tokens\n");

	Node root;
	root.children = arena_alloc(get_arena(), sizeof(Node) * NODE_ROOT_CHILDREN_COUNT);
	root.len = 0;

	parse(&root, tokens);
	//print_tree(&root);
	//print_tree(&root.children[1]);
	printf("parsed ast\n");

	if (!validate_types(&root)) {
		fprintf(stderr, "failed to validate types\n");
		arena_free(get_arena());
		exit(EXIT_FAILURE);
	}
	printf("validated types\n");

	generate_asm(&root, "build/a.asm");
	printf("generated assembly\n");

	system("nasm -f elf64 build/a.asm");
	system("ld build/a.o -o a.out");

	arena_free(get_arena());
	return 0;

	*/
}
