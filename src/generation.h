#include "parser.h"

#define STACK_SIZE 64
#define VARIABLE_NAME_SIZE 64

typedef struct {
	char name[VARIABLE_NAME_SIZE];
	int location; 
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
	if (node->type != NODE_EXPRESSION) {
		fprintf(stderr, "Expected expression node. :%i\n", 
				node->token->line);
		return;
	} if (!node->token->has_value) {
		fprintf(stderr, "Expression token has no value. :%i\n", 
				node->token->line);
		return;
	}

	switch (node->token->type) {
		case INT_LITERAL:
			fprintf(fh, "\tmov rax, %s\n", node->token->value);
			push(fh, "rax");
			break;

		case VARIABLE:
			int match = 0;
			for (int i = 0; i <= stack_index; i++) {
				if (strcmp(stack[i].name,
							node->token->value) == 0) {
					match = 1;
					fprintf(fh, "\tpush QWORD [rsp + %i]\n",
							(stack_index - stack[i].location - 1) * 8);
					stack_index++;
					break;
				}
			}
			if (!match) {
				fprintf(stderr, 
						"Variable name does not exist.. '%s' :%i\n",
						node->token->value,
						node->token->line);
				return;
			}
			break;

		default:
			fprintf(stderr, "Node type unsupported.\n");
			break;
	}

	// move expression into desired variable
	if (node->parent->token->type == VARIABLE && 
			node->parent->type != NODE_DECLARATION) {
		int match = 0;
		for (int i = 0; i <= stack_index; i++) {
			if (strcmp(stack[i].name,
						node->parent->token->value) == 0) {
				match = 1;
				pop(fh, "rax");
				fprintf(fh, "\tmov [rsp + %i], rax\n",
						(stack_index - stack[i].location - 1) * 8);
				break;
			}
		}
		if (!match) {
			fprintf(stderr, 
					"Variable name does not exist... '%s' :%i\n",
					node->parent->token->value,
					node->parent->token->line);
		}
	}
}

// takes in node with type operator
// perform operation
void generate_operator(FILE* fh, Node* node) {
	if (node->parent->type == NODE_DECLARATION) {
		// create variable on stack
		fprintf(fh, "\tmov rax, 0\n");
		push(fh, "rax");
	}

	// push both operands to the stack
	generate_expression(fh, &node->children[0]);
	generate_expression(fh, &node->children[1]);

	pop(fh, "rbx");
	pop(fh, "rax");

	if (node->token->type == ADD)
		fprintf(fh, "\tadd rax, rbx\n");
	else if (node->token->type == SUBTRACT)
		fprintf(fh, "\tsub rax, rbx\n");
	else if (node->token->type == MULTIPLY)
		fprintf(fh, "\tmul rax, rbx\n");
	else if (node->token->type == DIVIDE)
		fprintf(fh, "\tdiv rax, rbx\n");
	else
		fprintf(stderr, "Bowser Jr. :%i\n", 
				node->token->line);

	int match = 0;
	for (int i = 0; i <= stack_index; i++) {
		if (strcmp(stack[i].name, node->parent->token->value) == 0) {
			match = 1;
			fprintf(fh, "\tmov [rsp + %i], rax\n", 
				   (stack_index - stack[i].location - 1) * 8);
		}
	}
	if (!match) {
		fprintf(stderr, "Variable name does not exist. '%s' :%i\n",
				node->parent->token->value,
				node->token->line);
	}
}

void generate_statement(FILE*, Node*);

void generate_function(FILE* fh, Node* node) {
	printf("a\n");

	if (node->type != NODE_DECLARATION_FUNC) {
		fprintf(stderr, "Expected function declaration. :%i\n", 
				node->token->line);
		return;
	}

	fprintf(fh, "\n%s:\n", node->token->value);
	fprintf(fh, "\tenter 64, 0\n");

	for (int i = 0; i < node->len; i++) {
		if (node->type != NODE_STATEMENT && node->type != NODE_DECLARATION && 
				node->type != NODE_DECLARATION_FUNC) {
			fprintf(stderr, "Node in root is not a valid type. :%i\n", 
					node->token->line);
			return;
		}

		generate_statement(fh, &node->children[i]);
	}

	fprintf(fh, "\tleave\n");
	fprintf(fh, "\tret\n");
}

void generate_statement(FILE* fh, Node* node) {
	switch (node->token->type) {
		case EXIT:
			fprintf(fh, "\n");

			generate_expression(fh, &node->children[0]);

			fprintf(fh, "\tmov rax, 60\n");
			pop(fh, "rdi");
			fprintf(fh, "\tleave\n");
			fprintf(fh, "\tsyscall\n");

			break;
		case VARIABLE:
			if (!node->token->has_value) {
				fprintf(stderr, 
						"Variable token does not have a value. %i\n", 
						node->token->line);
				return;
			}

			if (node->type == NODE_DECLARATION) {
				// adds variable to compiler stack
				strcpy(stack[stack_index].name, node->token->value);
				stack[stack_index].location = stack_index;
			}

			if (node->children[0].type == NODE_EXPRESSION) {
				generate_expression(fh, &node->children[0]);
			} else if (node->children[0].type == NODE_OPERATOR) {
				generate_operator(fh, &node->children[0]);
			} else {
				fprintf(stderr, "Major shitballs 2. :%i\n",
						node->token->line);
			}

			break;
		case FUNC_NAME:
			generate_function(fh, node);
			
			break;
		default:
			fprintf(stderr, "Unsupported token type. :%i\n", 
					node->token->line);
			break;
	}
}

void generate_asm(Node* root, const char* out_path) {
	FILE* fh = fopen(out_path, "w");
	if (fh == NULL) {
		fprintf(stderr, "Failed to create output file.\n");
		return;
	}

	fprintf(fh, "global _start\n");
	stack_index = 0;

	for (int i = 0; i < root->len; i++) {
		Node* node = &root->children[i];

		if (node->type != NODE_STATEMENT && node->type != NODE_DECLARATION && 
				node->type != NODE_DECLARATION_FUNC) {
			fprintf(stderr, "Node in root is not a valid type. :%i\n", 
					node->token->line);
			return;
		}

		generate_statement(fh, node);
	}

	fprintf(fh, "\n_start:\n");
	fprintf(fh, "\tenter %i, 0\n", STACK_SIZE);  // setup stack
	fprintf(fh, "\n\tmov rax, 60\n");
	fprintf(fh, "\tmov rdi, 0\n");
	fprintf(fh, "\tleave\n");
	fprintf(fh, "\tsyscall\n");

	fclose(fh);
}
