#include "type_checker.h"

#define STACK_SIZE 64
#define VARIABLE_NAME_SIZE 32
#define MAX_FUNCTIONS 1024

typedef struct {
	char name[VARIABLE_NAME_SIZE];
	int offset;  // offset from rbp
	int size;  // size on stack in bytes
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

void get_register(const Node* node, char* reg, const int count) {
	const Type type = node->_type;
	if (type == I8 || type == UI8) {
		if (count == 1) {
			strcpy(reg, "al");
		} else {
			strcpy(reg, "bl");
		}
	}
	if (type == I16 || type == UI16) {
		if (count == 1) {
			strcpy(reg, "ax");
		} else {
			strcpy(reg, "bx");
		}
	}
	if (type == I32 || type == UI32) {
		if (count == 1) {
			strcpy(reg, "eax");
		} else {
			strcpy(reg, "ebx");
		}
	}
	if (type == I64 || type == UI64) {
		if (count == 1) {
			strcpy(reg, "rax");
		} else {
			strcpy(reg, "rbx");
		}
	}
}

// pushes a variable to the top of the stack
// returns the variable offset
/*int fetch_variable(FILE* fh, const Node* node) {
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
}*/

// returns the variable offset from rbp
int get_variable_offset(const Node* node) {
	int offset = -1;

	for (int i = 0; i <= stack.index; i++) {
		if (strcmp(stack.variables[i].name,
					node->token->value) == 0) {
			offset = stack.variables[i].offset;
			return offset;
		}
	}

	fprintf(stderr,
			"Variable name does not exist.. '%s' :%i\n",
			node->token->value,
			node->token->line);

	return offset;
}

// an expression is either a literal or a variable
// puts value in register
void generate_expression(FILE* fh, const Node* node, const char* reg) {
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
		case TOK_INT_LITERAL:
			fprintf(fh, "\tmov %s, %s\n", reg, node->token->value);

			break;
		case TOK_VARIABLE:
			const int offset = get_variable_offset(node);
			if (reg[0] == '[') {
				fprintf(fh, "\tmov rax, [rbp - %i]\n", offset);
				fprintf(fh, "\tmov %s, rax\n", reg);
			} else {
				fprintf(fh, "\tmov %s, [rbp - %i]\n", reg, offset);
			}
			break;

		default:
			fprintf(stderr, "Node type unsupported.\n");
			break;
	}
}

// takes in node with type operator
// perform operation
void generate_operator(FILE* fh, const Node* node) {
	char reg1[4];
	char reg2[4];
	get_register(node, reg1, 1);
	get_register(node, reg2, 2);

	// TODO: potentially grabs wrong bytes from variable target
	generate_expression(fh, &node->children[0], reg1);
	generate_expression(fh, &node->children[1], reg2);

	// handle idiv and imul
	if (node->token->type == TOK_ADD)
		fprintf(fh, "\tadd %s, %s\n", reg1, reg2);
	else if (node->token->type == TOK_SUBTRACT)
		fprintf(fh, "\tsub %s, %s\n", reg1, reg2);
	else if (node->token->type == TOK_MULTIPLY)
		fprintf(fh, "\tmul %s, %s\n", reg1, reg2);
	else if (node->token->type == TOK_DIVIDE)
		fprintf(fh, "\tdiv %s, %s\n", reg1, reg2);
	else
		fprintf(stderr, "Bowser Jr. :%i\n",
				node->token->line);

	const int offset = get_variable_offset(node->parent);
	fprintf(fh, "\tmov [rbp - %i], rax\n", offset);
}

void generate_statement(FILE*, const Node*);

void generate_function(FILE* fh, const Node* node) {
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
		generate_statement(fh, &node->children[i]);
		if (node->children[i].type == NODE_LOCAL_VARIABLE) {
			if (i == 0)
				fprintf(fh, "\tmov [rbp - %i], rax\n", get_variable_offset(&node->children[i]));
			if (i == 1)
				fprintf(fh, "\tmov [rbp - %i], rbx\n", get_variable_offset(&node->children[i]));
		}
	}

	fprintf(fh, "\n\tmov rsp, rbp\n");
	fprintf(fh, "\tpop rbp\n");
	fprintf(fh, "\tret\n");
}

void generate_exit(FILE* fh, const Node* node) {
	fprintf(fh, "\n");

	generate_expression(fh, &node->children[0], "rdi");

	fprintf(fh, "\tmov rax, 60\n");
	fprintf(fh, "\tsyscall\n");
}

void generate_return(FILE* fh, const Node* node) {
	fprintf(fh, "\n");
	
	generate_expression(fh, &node->children[0], "rax");
}

void generate_function_call(FILE* fh, const Node* node) {
	for (int i = 0; i < node->len; i++) {
		const Node* child = &node->children[i];
		if (i == 0)
			generate_expression(fh, child, "rax");
		if (i == 1)
			generate_expression(fh, child, "rbx");

		/*
		// TODO: stack based argument passing
		generate_expression(fh, child, "rax");
		push(fh, "rax");
		*/
	}

	fprintf(fh, "\tcall %s\n", node->token->value);

	if (node->parent->token->type == TOK_VARIABLE) {
		const int offset = get_variable_offset(node->parent);
		fprintf(fh, "\tmov [rbp - %i], rax\n", offset);
	}
}

void generate_struct(FILE*fh, const Node* node) {
	int previous_offset = 0;
	if (stack.count[stack._index] != 0) {
		previous_offset = stack.variables[stack.index - 1].offset;
	}

	const Type type = node->_type;

	for (int i = 0; i < struct_tracker.i; i++) {
		if (type_lookup(struct_tracker.nodes[i]->token) == type) {
			const Node* struct_node = struct_tracker.nodes[i];

			int struct_size = 0;
			for (int j = 0; j < struct_node->len; j++) {
				const Token* member_token = struct_node->children[j].token;

				char temp[32] = {};
				sprintf(temp, "%s.%s", node->token->value, member_token->value);

				// adds variable to compiler stack
				strcpy(stack.variables[stack.index].name,
						temp);
				stack.variables[stack.index].size = 8;
				stack.variables[stack.index].offset = previous_offset + 8;

				struct_size += stack.variables[stack.index].size;

				stack.index++;
				stack.count[stack._index]++;

				previous_offset = stack.variables[stack.index - 1].offset;
			}

			fprintf(fh, "\tsub rsp, %i  ; %s\n",
				struct_size, node->token->value);
		}
	}
}

void generate_statement(FILE* fh, const Node* node) {
	switch (node->token->type) {
		case TOK_EXIT:
			generate_exit(fh, node);

			break;
		case TOK_VARIABLE:
			if (!node->token->has_value) {
				fprintf(stderr, 
						"Variable token does not have a value. %i\n", 
						node->token->line);
				return;
			}

			if (node->type == NODE_DECLARATION || node->type == NODE_LOCAL_VARIABLE) {
				// get size of variable
				int previous_offset = 0;
				if (stack.count[stack._index] != 0) {
					previous_offset = stack.variables[stack.index - 1].offset;
				}

				if (node->_type >= STRUCT1) {
					generate_struct(fh, node);
					return;
				}

				// adds variable to compiler stack
				strcpy(stack.variables[stack.index].name,
						node->token->value);
				stack.variables[stack.index].size = 8;
				stack.variables[stack.index].offset = previous_offset + 8;
				/*stack.variables[stack.index].offset =
					(stack.count[stack._index] + 1) * 8;*/

				fprintf(fh, "\tsub rsp, %i  ; %s\n",
					stack.variables[stack.index].size,
					stack.variables[stack.index].name);
				stack.index++;
				stack.count[stack._index]++;
			}

			if (node->type == NODE_LOCAL_VARIABLE) {
				return;
			}

			if (node->children[0].type == NODE_EXPRESSION) {
				const int offset = get_variable_offset(node);
				char reg[32] = {};
				strcat(reg, "[rbp - ");
				sprintf(reg + strlen(reg), "%i", offset);
				strcat(reg, "]");

				generate_expression(fh, &node->children[0], reg);

			} else if (node->children[0].type == NODE_OPERATOR) {
				generate_operator(fh, &node->children[0]);

			} else if (node->children[0].type == NODE_CALL_FUNC) {
				generate_function_call(fh, &node->children[0]);

			} else {
				fprintf(stderr, "Major shitballs 2. Type '%d' :%i\n",
						node->type,
						node->token->line);
			}

			break;
		case TOK_FUNC_NAME:
			if (node->type == NODE_CALL_FUNC) {
				generate_function_call(fh, node);
			} else { 
				generate_function(fh, node);
			}

			break;
		case TOK_RETURN:
			generate_return(fh, node);

			break;
		case TOK_TYPE:
			// ignore
			break;
		default:
			fprintf(stderr, "Unsupported token type. '%d' :%i\n",
					node->token->type,
					node->token->line);
			break;
	}
}

void generate_asm(const Node* root, const char* out_path) {
	FILE* fh = fopen(out_path, "w");
	if (fh == NULL) {
		fprintf(stderr, "Failed to create output file.\n");
		return;
	}

	fprintf(fh, "global _start\n");
	stack.index = 0;
	stack._index = 0;

	for (int i = 0; i < root->len; i++) {
		const Node* node = &root->children[i];

		if (node->type != NODE_STATEMENT && node->type != NODE_DECLARATION && 
				node->type != NODE_DECLARATION_FUNC && node->type != NODE_STRUCT) {
			fprintf(stderr, "Node in root is not a valid type. :%i\n", 
					node->token->line);
			return;
		}

		generate_statement(fh, node);
	}

	for (int i = 0; i < stack.index; i++) {
		printf("name: '%s', %i\n", stack.variables[i].name, stack.variables[i].size);
	}

	fprintf(fh, "\n_start:\n");
	fprintf(fh, "\tmov rbp, rsp\n\n");

	fprintf(fh, "\tcall main\n");

	fprintf(fh, "\n\tmov rdi, rax\n");
	fprintf(fh, "\tmov rax, 60\n");
	fprintf(fh, "\tsyscall\n");

	fclose(fh);
}
