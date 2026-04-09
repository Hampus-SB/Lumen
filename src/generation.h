#include "parser.h"

#define STACK_SIZE 64
#define VARIABLE_NAME_SIZE 64

typedef struct {
	char name[VARIABLE_NAME_SIZE];
	int offset;  // byte offset
} Variable;

Variable stack[STACK_SIZE];
int stack_index;

void push(FILE* fh, const char* reg) {
	fprintf(fh, "\tpush %s\n", reg);
	stack_index++;
}

void pop(FILE* fh, const char* reg) {
	fprintf(fh, "\tpop %s\n", reg);
	stack_index--;
}

// an expression is either a literal or a variable
// put value in stack
void generate_expression(FILE* fh, Node* node) {
	if (node->type != EXPRESSION) {
		fprintf(stderr, "Expected expression node.\n");
		return;
	} if (!node->token->has_value) {
		fprintf(stderr, "Expression token has no value.\n");
		return;
	}

	switch (node->token->type) {
		case INT_LITERAL:
			fprintf(fh, "\tmov rax, %s\n", node->token->value);
			push(fh, "rax");
			break;

		case VARIABLE:
			int match = 0;
			for (int i = 0; i < stack_index; i++) {
				if (strcmp(stack[i].name,
							node->token->value) == 0) {
					match = 1;
					fprintf(fh, "\tpush QWORD [rsp + %i]\n",
							stack[i].offset);
					break;
				}
			}
			if (!match) {
				fprintf(stderr, "Variable name does not exist.\n");
				return;
			}
			break;

		default:
			fprintf(stderr, "Node type unsupported.\n");
			break;
	}
}

void generate_statement(FILE* fh, Node* node) {
	switch (node->token->type) {
		case EXIT:
			generate_expression(fh, &node->children[0]);

			fprintf(fh, "\tmov rax, 60\n");
			pop(fh, "rdi");
			fprintf(fh, "\tleave\n");
			fprintf(fh, "\tsyscall\n");

			break;
		case VARIABLE:
			if (!node->token->has_value) {
				fprintf(stderr, 
						"Variable token does not have a value\n");
				return;
			}

			strcpy(stack[stack_index].name, node->token->value);
			stack[stack_index].offset = stack_index * 8 + 8;

			generate_expression(fh, &node->children[0]);

			break;
		default:
			fprintf(stderr, "Unsupported token type.\n");
			break;
	}
}

void generate_asm(NodeRoot* root, const char* out_path) {
	FILE* fh = fopen(out_path, "w");
	if (fh == NULL) {
		fprintf(stderr, "Failed to create output file.\n");
		return;
	}

	fprintf(fh, "global _start\n_start:\n");
	fprintf(fh, "\tenter %i, 0\n", STACK_SIZE);  // setup stack

	if (root->len < 1) {
		// TODO: something
		return;
	}

	stack_index = 0;

	for (int i = 0; i < root->len; i++) {
		Node* node = &root->children[i];

		if (node->type != STATEMENT) {
			fprintf(stderr, "Node in root is not a statement.\n");
			return;
		}

		generate_statement(fh, node);
	}

	fclose(fh);
}
