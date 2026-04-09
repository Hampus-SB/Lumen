#include "parser.h"

#define STACK_SIZE 64
#define VARIABLE_NAME_SIZE 64

typedef struct {
	char name[VARIABLE_NAME_SIZE];
	int location;  // byte offset
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
		//printf(": %d, %d\n", node->type, node->token.type);

		if (node->type != STATEMENT) {
			fprintf(stderr, "Whoops.\n");
			return;
		}
		
		if (node->token->type == EXIT) {
			Node* child = &node->children[0];

			if (!child->token->has_value) {
				fprintf(stderr, "Token has no value.\n");
				return;
			}

			// push expr onto stack
			if (child->token->type == VARIABLE) {
				int match = 0;
				for (int i = 0; i < stack_index; i++) {
					if (strcmp(stack[i].name, child->token->value) == 0) {
					 	match = 1;
						fprintf(fh, 
								"\tpush QWORD [rsp + %i]\n", 
								stack[i].location);
						break;
					}
				}
				if (!match) {
					fprintf(stderr, "Variable name does not exist.\n");
				}
			} else if (child->token->type == INT_LITERAL) {
				// push int literal onto stack
				fprintf(fh, "\tmov rax, %s\n", child->token->value);
				push(fh, "rax");
			} else {
				fprintf(stderr, "Unexpected token type.\n");
				return;
			}

			fprintf(fh, "\tmov rax, 60\n");
			pop(fh, "rdi");
			fprintf(fh, "\tleave\n");
			fprintf(fh, "\tsyscall\n");
		}

		else if (node->token->type == VARIABLE) {
			Node* node_expr = &node->children[0];
			if (node_expr->type != EXPRESSION) {
				fprintf(stderr, "Expected expression node.\n");
				return;
			}
			if (!node_expr->token->has_value) {
				fprintf(stderr, "Expression token has no value.\n");
				return;
			}

			// put new variable inside compile time stack
			printf(": stack index: %i\n", stack_index);
			strcpy(stack[stack_index].name, node->token->value);
			stack[stack_index].location = stack_index * 8 + 8;
			
			// push variable onto asm stack
			fprintf(fh, "\tmov rax, %s\n", node_expr->token->value);
			push(fh, "rax");
		}

		else {
			fprintf(stderr, "Unsupported node in root hierarchy.\n");
		}
	}

	fclose(fh);
}

