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

	char file[FILE_BUFFER_SIZE];
	memset(file, 0, FILE_BUFFER_SIZE);

	char buffer[FILE_BUFFER_LINE_SIZE];
	while (fgets(buffer, FILE_BUFFER_LINE_SIZE, fh) != NULL) {
		strcat(file, buffer);
	}

	fclose(fh);

	Token** tokens = calloc(TOKEN_COUNT, sizeof(Token));
	int count = tokens_from_source(file, tokens);
	for (int i = 0; i < count; i++) {
		//print_token(tokens[i]);
	}
	printf("generated tokens\n");

	Node root;
	root.children = malloc(sizeof(Node) * NODE_ROOT_CHILDREN_COUNT);
	root.len = 0;
	parse(&root, tokens, count);
	
	printf("parsed ast\n");

	//print_tree(&root);
	//printf("|\n");
	//print_tree(&root.children[0]);
	
	generate_asm(&root, "build/a.asm");

	printf("generated assembly\n");

	system("nasm -felf64 build/a.asm");
	system("ld build/a.o -o a.out");

	for (int i = 0; i < count; i++) {
		free(tokens[i]);
	}
	free(tokens);
	free_tree(&root);

	return 0;
}
