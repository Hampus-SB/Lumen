#include "type_checker.h"

#define STACK_SIZE 64
#define VARIABLE_NAME_SIZE 64
#define MAX_FUNCTIONS 1024

typedef struct {
	char name[VARIABLE_NAME_SIZE];
	int offset;  // offset from rbp
	Type type;
} Variable;

typedef struct {
	Variable variables[STACK_SIZE];
	int index;

	// how many variables in i stack
	int count[MAX_FUNCTIONS];
	int _index;  // which 'stack'
} Stack;

Stack stack;

void push(FILE* fh, const char* reg) {
	fprintf(fh, "\tpush %s\n", reg);
	stack.index++;
	stack.count[stack._index]++;
}

void pop(FILE* fh, const char* reg) {
	fprintf(fh, "\tpop %s\n", reg);
	stack.index--;
	stack.count[stack._index]--;
}

// pushes a variable to the top of the stack
// returns the variable offset
int fetch_variable(FILE* fh, Node* node) {
	int offset = -1;

	int match = 0;
	for (int i = 0; i <= stack.index; i++) {
		if (strcmp(stack.variables[i].name,
					node->token->value) == 0) {
			match = 1;
			fprintf(fh, "\tpush QWORD [rbp - %i]\n", 
					stack.variables[i].offset);

			offset = stack.variables[i].offset;
			stack.index++;
			stack.count[stack._index]++;
			break;
		}
	}

	if (!match) {
		fprintf(stderr, 
				"Variable name does not exist.. '%s' :%i\n",
				node->token->value,
				node->token->line);
	}

	return offset;
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
			// push variable to stack
			int offset = fetch_variable(fh, node);
			
			// move variable into destination
			if (node->parent->token->type == VARIABLE && 
					node->parent->type != NODE_DECLARATION) {
				pop(fh, "rax");
				fprintf(fh, "\tmov [rbp - %i], rax\n", offset);
			}
			break;

		default:
			fprintf(stderr, "Node type unsupported.\n");
			break;
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


	int offset = fetch_variable(fh, node->parent);
	fprintf(fh, "\tmov [rbp - %i], rax\n", offset);
	
	// fetch_variable pushes to stack but we dont want that here
	pop(fh, "rax");
}

void generate_statement(FILE*, Node*);

void generate_function(FILE* fh, Node* node) {
	if (node->type != NODE_DECLARATION_FUNC) {
		fprintf(stderr, "Expected function declaration. :%i\n", 
				node->token->line);
		return;
	}

	stack._index++;

	fprintf(fh, "\n%s:\n", node->token->value);
	fprintf(fh, "\tpush rbp\n");
	fprintf(fh, "\tmov rbp, rsp\n\n");

	for (int i = 0; i < node->len; i++) {
		if (node->type != NODE_STATEMENT && node->type != NODE_DECLARATION && 
				node->type != NODE_DECLARATION_FUNC) {
			fprintf(stderr, "Node in root is not a valid type. :%i\n", 
					node->token->line);
			return;
		}

		generate_statement(fh, &node->children[i]);
	}

	fprintf(fh, "\n\tmov rsp, rbp\n");
	fprintf(fh, "\tpop rbp\n");
	fprintf(fh, "\tret\n");

	//stack._index--;
}

void generate_exit(FILE* fh, Node* node) {
	fprintf(fh, "\n");

	generate_expression(fh, &node->children[0]);

	fprintf(fh, "\tmov rax, 60\n");
	pop(fh, "rdi");
	fprintf(fh, "\tsyscall\n");
}

void generate_return(FILE* fh, Node* node) {
	fprintf(fh, "\n");
	
	generate_expression(fh, &node->children[0]);
	pop(fh, "rax");
}

void generate_function_call(FILE* fh, Node* node) {
	if (node->parent->type == NODE_DECLARATION) {
		fprintf(fh, "\tmov rax, 0\n");
		push(fh, "rax");
	}

	fprintf(fh, "\tcall %s\n", node->token->value);

	if (node->parent->token->type == VARIABLE) {
		int offset = fetch_variable(fh, node->parent);
		pop(fh, "rbx");  // we dont want variable on stack
		fprintf(fh, "\tmov [rbp - %i], rax\n", offset);
	}
}

void generate_statement(FILE* fh, Node* node) {
	switch (node->token->type) {
		case EXIT:
			generate_exit(fh, node);

			break;
		case VARIABLE:
			printf("||| %d\n", node->children[0].type);

			if (!node->token->has_value) {
				fprintf(stderr, 
						"Variable token does not have a value. %i\n", 
						node->token->line);
				return;
			}

			if (node->type == NODE_DECLARATION) {
				printf("count: %i, '%s'\n", 
						stack._index, node->token->value);

				// adds variable to compiler stack
				strcpy(stack.variables[stack.index].name, 
						node->token->value);

				stack.variables[stack.index].offset = 
					(stack.count[stack._index] + 1) * 8;
			}

			if (node->children[0].type == NODE_EXPRESSION) {
				generate_expression(fh, &node->children[0]);
			} else if (node->children[0].type == NODE_OPERATOR) {
				generate_operator(fh, &node->children[0]);
			} else if (node->children[0].type == NODE_CALL_FUNC) {
				generate_function_call(fh, &node->children[0]);
			} else {
				fprintf(stderr, "Major shitballs 2. :%i\n",
						node->token->line);
			}

			break;
		case FUNC_NAME:
			if (node->type == NODE_CALL_FUNC) {
				generate_function_call(fh, node);
			} else { 
				generate_function(fh, node);
			}

			break;
		case RETURN:
			generate_return(fh, node);

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
	stack.index = 0;
	stack._index = 0;

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
	fprintf(fh, "\tmov rbp, rsp\n\n");

	// call main
	fprintf(fh, "\tcall main\n");
	
	fprintf(fh, "\n\tmov rax, 60\n");
	fprintf(fh, "\tmov rdi, 0\n");
	fprintf(fh, "\tsyscall\n");

	fclose(fh);
}
