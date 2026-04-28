#include "generation.h"

#define FILE_BUFFER_SIZE 8192
#define FILE_BUFFER_LINE_SIZE 128

int main(int argc, char** argv) {
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

	arena_init(&arena, ARENA_DEFAULT_SIZE);

	TokenArray tokens;
	tokens.capacity = TOKEN_DEFAULT_COUNT;
	tokens.count = 0;
	tokens.tokens = arena_alloc(&arena, tokens.capacity * sizeof(Token));

	tokens_from_source(file, &tokens);

	for (int i = 0; i < tokens.count; i++) {
		token_print(&tokens.tokens[i]);
	}

	printf("generated tokens\n");

	Node root;
	root.children = malloc(sizeof(Node) * NODE_ROOT_CHILDREN_COUNT);
	root.len = 0;

	parse(&root, tokens);

	printf("parsed ast\n");
	
	generate_asm(&root, "build/a.asm");

	printf("generated assembly\n");

	system("nasm -felf64 build/a.asm");
	system("ld build/a.o -o a.out");

	arena_free(&arena);

	return 0;
}
