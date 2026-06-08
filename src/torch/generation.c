#include "../../include/torch/generation.h"
#include "../../include/torch/symbols.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "logging.h"

Stack stack;

void get_label(char* out) {
	static int count = 0;
	sprintf(out, "L%i", count++);
}

void stack_init() {
	stack.index = 0;
	for (int i = 0; i < MAX_FUNCTIONS; i++) {
		stack.count[i] = 0;
	}
	stack._index = 0;
}

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

	logerror("Variable name does not exist. '%s' :%i",
			node->token->value,
			node->token->line);

	return offset;
}

// returns the variable offset from base of struct
int get_variable_struct_offset(const Node* node) {
	int offset = -1;

	for (int i = 0; i <= stack.index; i++) {
		if (strcmp(stack.variables[i].name,
					node->token->value) == 0) {
			offset = stack.variables[i].struct_offset;
			return offset;
		}
	}

	logerror("Variable name does not exist. '%s' :%i",
			node->token->value,
			node->token->line);

	return offset;
}

// an expression is either a literal or a variable
// puts value in register
void generate_expression(FILE* fh, const Node* node, const char* reg) {
	if (node->type != NODE_EXPRESSION && node->type != NODE_INDEX) {
		logerror("Expected expression node. :%i",
				node->token->line);
		return;
	}
	if (!node->token->has_value) {
		logerror("Expression token has no value. :%i",
				node->token->line);
		return;
	}

	switch (node->token->type) {
		case TOK_INT_LITERAL:
			fprintf(fh, "\tmov %s, qword %s\n", reg, node->token->value);

			break;
		case TOK_VARIABLE:
			const int offset = get_variable_offset(node);

			// index operator []
			if (node->len == 1) {
				//char* index = node->children[0].token->value;
				generate_expression(fh, &node->children[0], "rax");

				fprintf(fh, "\timul rax, %i\n", 1);
				fprintf(fh, "\tadd rax, [rbp - %i]\n", offset);

				if (reg[0] == '[') {
					fprintf(fh, "\tmov rax, [rax]\n");
					fprintf(fh, "\tmov %s, rax\n", reg);
				} else {
					fprintf(fh, "\tmov %s, [rax]\n", reg);
				}

				return;
			}

			// node type_info is a pointer but the variable is not (&var)
			if (types_is_ptr(node->type_info)) {
				if (types_is_ptr(symbol_table_find_type(node->token->value))) {
					// variable is an address
					fprintf(fh, "\tmov rax, [rbp - %i]\n", offset);
					fprintf(fh, "\tmov %s, rax\n", reg);
				} else {
					// take address of variable
					fprintf(fh, "\tlea rax, [rbp - %i]\n", offset);
					if (strcmp(reg, "rax") != 0)
						fprintf(fh, "\tmov %s, rax\n", reg);
				}
				return;
			}

			// node type_info is not a pointer but the variable is (*var)
			if (types_is_ptr(symbol_table_find_type(node->token->value))) {
				fprintf(fh, "\tmov rcx, [rbp - %i]\n", offset);
				if (reg[0] == '[') {
					fprintf(fh, "\tmov rdx, [rcx]\n");
					fprintf(fh, "\tmov %s, rdx\n", reg);
				} else {
					fprintf(fh, "\tmov %s, [rcx]\n", reg);
				}
				return;
			}

			if (reg[0] == '[') {
				fprintf(fh, "\tmov rax, [rbp - %i]\n", offset);
				fprintf(fh, "\tmov %s, rax\n", reg);
			} else {
				fprintf(fh, "\tmov %s, [rbp - %i]\n", reg, offset);
			}
			break;

		default:
			logerror("Node type unsupported.");
			break;
	}
}

// takes in node with type operator
// perform operation
void generate_operator(FILE* fh, const Node* node) {
	const char* reg1 = register_from_type_obj(node->type_info, 1);
	const char* reg2 = register_from_type_obj(node->type_info, 2);

	// TODO: potentially grabs wrong bytes from variable target
	generate_expression(fh, &node->children[0], "rax");
	generate_expression(fh, &node->children[1], "rbx");

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
		logerror("Bowser Jr. :%i",
				node->token->line);

	const int offset = get_variable_offset(node->parent);
	fprintf(fh, "\tmov [rbp - %i], rax\n", offset);
}

void generate_node(FILE*, const Node*);

void generate_function(FILE* fh, const Node* node) {
	if (node->type != NODE_DECLARATION_FUNC) {
		logerror("Expected function declaration. :%i",
				node->token->line);
		return;
	}

	stack._index++;

	fprintf(fh, "\n%s:\n", node->token->value);
	fprintf(fh, "\tpush rbp\n");
	fprintf(fh, "\tmov rbp, rsp\n\n");

	for (int i = 0; i < node->len; i++) {
		generate_node(fh, &node->children[i]);
		if (node->children[i].type == NODE_LOCAL_VARIABLE) {
			if (i == 0)
				fprintf(fh, "\tmov [rbp - %i], rax\n", get_variable_offset(&node->children[i]));
			if (i == 1)
				fprintf(fh, "\tmov [rbp - %i], rbx\n", get_variable_offset(&node->children[i]));
		} else {
			logwarning("Expected local variable node. '%d' :%i",
				node->type,
				node->token->line);
		}
	}
	fprintf(fh, "\n\tmov rsp, rbp\n");
	fprintf(fh, "\tpop rbp\n");
	fprintf(fh, "\tret\n");
}

void generate_return(FILE* fh, const Node* node) {
	fprintf(fh, "\n");

	generate_expression(fh, &node->children[0], "rax");

	// exit the function
	fprintf(fh, "\tmov rsp, rbp\n");
	fprintf(fh, "\tpop rbp\n");
	fprintf(fh, "\tret\n");
}

void generate_function_call(FILE* fh, const Node* node) {
	for (int i = 0; i < node->len; i++) {
		const Node* child = &node->children[i];

		// holy slop
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
		if (types_is_ptr(node->parent->type_info)) {
			const int struct_offset = get_variable_struct_offset(node->parent);
			fprintf(fh, "\tmov rbx, [rbp - %i]\n", offset);
			fprintf(fh, "\tsub rbx, %i\n", struct_offset);
			fprintf(fh, "\tmov [rbx], rax\n");
		}
		else {
			fprintf(fh, "\tmov [rbp - %i], rax\n", offset);
		}
	}
}

void generate_struct(FILE* fh, const Node* node) {
	int previous_offset = 0;
	if (stack.count[stack._index] != 0) {
		previous_offset = stack.variables[stack.index - 1].offset;
	}

	const TypeObj* type = node->type_info;
	const Node* struct_node = symbol_table_find_struct(type);
	int struct_size = 0;

	// adds variable to compiler stack
	strcpy(stack.variables[stack.index].name,
			node->token->value);
	stack.variables[stack.index].size = 8;
	stack.variables[stack.index].offset = previous_offset + 8;
	if (types_is_ptr(type)) {
		previous_offset += 8;
	}

	stack.index++;
	stack.count[stack._index]++;

	for (int i = 0; i < struct_node->len; i++) {
		const Token* member_token = struct_node->children[i].token;

		char temp[TOKEN_BUFFER_SIZE] = {};
		if (types_is_ptr(type)) {
			sprintf(temp, "%s->%s", node->token->value, member_token->value);

			// adds variable to compiler stack
			strcpy(stack.variables[stack.index].name,
					temp);
			stack.variables[stack.index].size = 0;
			stack.variables[stack.index].offset = previous_offset;
			stack.variables[stack.index].struct_offset = i * 8;

			stack.index++;
			stack.count[stack._index]++;
		} else {
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
	}

	if (types_is_ptr(type)) {
		struct_size = 8;
	}

	fprintf(fh, "\tsub rsp, %i  ; %s\n", struct_size,
		node->token->value);
}

void generate_assembly_node(FILE* fh, const Node* node) {
	fprintf(fh, "%s", node->token->value);
}

// adds the variable to the compiler stack and allocates on the asm stack
void generate_variable_declaration(FILE* fh, const Node* node) {
	if (types_is_struct(node->type_info)) {
		generate_struct(fh, node);
		return;
	}

	// get size of variable
	int previous_offset = 0;
	if (stack.count[stack._index] != 0) {
		previous_offset = stack.variables[stack.index - 1].offset;
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

// TODO: rename this function
// indexing into arrays, ex. var[0] = 5;
void generate_index(FILE* fh, const Node* node) {
	const int offset = get_variable_offset(node->parent);

	generate_expression(fh, node, "rax");
	generate_expression(fh, &node->parent->children[1], "rbx");

	fprintf(fh, "\timul rax, %i\n", 1);
	fprintf(fh, "\tadd rax, [rbp - %i]\n", offset);
	fprintf(fh, "\tmov [rax], qword rbx\n");
}

// node is "variable" node type
void generate_right_side(FILE* fh, const Node* node) {
	const int offset = get_variable_offset(node);
	char reg[32] = {};
	strcat(reg, "[rbp - ");
	sprintf(reg + strlen(reg), "%i", offset);
	strcat(reg, "]");

	if (types_is_ptr(symbol_table_find_type(node->token->value)) &&
			!types_is_ptr(node->type_info)) {
		// pointer dereference
		fprintf(fh, "\tmov [rbp - %i], rax\n", offset);
		generate_expression(fh, &node->children[0], "[rax]");
	}
	else if (types_is_arrow(node->token->value)) {
		const int struct_offset = get_variable_struct_offset(node);
		fprintf(fh, "\tmov rax, [rbp - %i]\n", offset);
		fprintf(fh, "\tsub rax, %i\n", struct_offset);

		// rax contains the memory address of the member we want to access
		generate_expression(fh, &node->children[0], "[rax]");
	}
	else {
		generate_expression(fh, &node->children[0], reg);
	}
}

void generate_if_statement(FILE* fh, const Node* node) {
	fprintf(fh, "\n");

	// condition
	generate_expression(fh, &node->children[0].children[0], "rbx");
	generate_expression(fh, &node->children[0].children[1], "rax");

	char label[32] = {0};
	get_label(label);

	fprintf(fh, "\tcmp rbx, rax\n");

	switch (node->children[0].token->type) {
		case TOK_EQUIVALENT:
			fprintf(fh, "\tjne %s\n\n", label);
			break;

		case TOK_GREATERTHAN:
			fprintf(fh, "\tjle %s\n\n", label);
			break;

		case TOK_LESSTHAN:
			fprintf(fh, "\tjge %s\n\n", label);
			break;

		case TOK_GREATERTHANEQUAL:
			fprintf(fh, "\tjl %s\n\n", label);
			break;

		case TOK_LESSTHANEQUAL:
			fprintf(fh, "\tjg %s\n\n", label);
			break;

		default:
			logwarning("If statement has a wierd token.");
			break;
	}

	// start at 1 to skip the boolean expression node
	for (int i = 1; i < node->len; i++) {
		generate_node(fh, &node->children[i]);
	}

	fprintf(fh, "\n\tjmp %s\n", label);

	// end label
	fprintf(fh, "\n%s:\n", label);
}

void generate_while_loop(FILE* fh, const Node* node) {
	fprintf(fh, "\n");

	char label_start[32];
	char label_end[32];
	get_label(label_start);
	get_label(label_end);

	fprintf(fh, "%s:\n", label_start);

	// comparison
	// if the condition isnt true jump to the end of the while block
	generate_expression(fh, &node->children[0].children[0], "rbx");
	generate_expression(fh, &node->children[0].children[1], "rax");

	fprintf(fh, "\tcmp rbx, rax\n");

	// TODO: extract into function to avoid duplicate
	switch (node->children[0].token->type) {
		case TOK_EQUIVALENT:
			fprintf(fh, "\tjne %s\n", label_end);
			break;

		case TOK_GREATERTHAN:
			fprintf(fh, "\tjle %s\n", label_end);
			break;

		case TOK_LESSTHAN:
			fprintf(fh, "\tjge %s\n", label_end);
			break;

		case TOK_GREATERTHANEQUAL:
			fprintf(fh, "\tjl %s\n", label_end);
			break;

		case TOK_LESSTHANEQUAL:
			fprintf(fh, "\tjg %s\n", label_end);
			break;

		default:
			logwarning("While loop has a wierd token.");
			break;
	}

	// start at 1 to skip the boolean expression node
	for (int i = 1; i < node->len; i++) {
		switch (node->children[i].type) {
			case NODE_BREAK:
				fprintf(fh, "\tjmp %s\n", label_end);
				break;
			case NODE_CONTINUE:
				fprintf(fh, "\tjmp %s\n", label_start);
				break;
			default:
				generate_node(fh, &node->children[i]);
				break;
		}
	}

	fprintf(fh, "\tjmp %s\n\n", label_start);
	fprintf(fh, "%s:\n", label_end);
}

void generate_node(FILE* fh, const Node* node) {
	//printf("gen node: type = %d\n", node->type);

	switch (node->type) {
		case NODE_IGNORE: case NODE_STRUCT:
			break;

		case NODE_EXPRESSION:
			logwarning("This should not happen 1.");
			generate_expression(fh, node, "oopsie");
			break;

		case NODE_OPERATOR:
			generate_operator(fh, node);
			break;

		case NODE_INDEX:
			generate_index(fh, node);
			break;

		case NODE_ASSEMBLY:
			generate_assembly_node(fh, node);
			break;

		case NODE_RETURN:
			generate_return(fh, node);
			break;

		case NODE_CALL_FUNC:
			generate_function_call(fh, node);
			break;

		case NODE_DECLARATION_FUNC:
			generate_function(fh, node);
			break;

		case NODE_LOCAL_VARIABLE:
			generate_variable_declaration(fh, node);
			break;

		case NODE_IF:
			generate_if_statement(fh, node);
			break;

		case NODE_WHILE:
			generate_while_loop(fh, node);
			break;

		case NODE_DECLARATION:
			generate_variable_declaration(fh, node);
			if (!types_is_struct(node->type_info)) {
				if (node->children[0].type != NODE_EXPRESSION) {
					generate_node(fh, &node->children[0]);
				} else {
					generate_right_side(fh, node);
				}
			}
			else {
				// wonky
				if (types_is_ptr(node->type_info)) {
					generate_right_side(fh, node);
				}
			}
			break;

		case NODE_STATEMENT:
			// is a statement guaranteed to have children?
			if (node->children[0].type != NODE_EXPRESSION) {
				generate_node(fh, &node->children[0]);
			} else {
				generate_right_side(fh, node);
			}
			break;

		default:
			logwarning("This should not happen 2. node type = %d", node->type);
			break;
	}
}

void generate_asm(const Node* root, const char* out_path) {
	FILE* fh = fopen(out_path, "w");
	if (fh == NULL) {
		logerror("Failed to create output file.");
		return;
	}

	fprintf(fh, "global _start\n");
	stack_init();

	for (int i = 0; i < root->len; i++) {
		const Node* node = &root->children[i];

		// TODO: might be redundant
		if (node->type != NODE_STATEMENT && node->type != NODE_DECLARATION && 
				node->type != NODE_DECLARATION_FUNC && node->type != NODE_STRUCT &&
				node->type != NODE_ASSEMBLY) {
			logerror("Node in root is not a valid type. '%d' :%i",
				node->type,
				node->token->line);
			return;
		}

		generate_node(fh, node);
	}

	fprintf(fh, "\n_start:\n");
	fprintf(fh, "\tmov rbp, rsp\n\n");

	fprintf(fh, "\tcall main\n");

	fprintf(fh, "\n\tmov rdi, rax\n");
	fprintf(fh, "\tmov rax, 60\n");
	fprintf(fh, "\tsyscall\n");

	fclose(fh);
}
