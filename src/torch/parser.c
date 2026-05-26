#include "../../include/torch/parser.h"
#include "../../include/torch/arena.h"
#include "../../include/torch/symbols.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	TokenArray tokens;
	int i;
} Parser;

Parser parser;

void parser_init(const TokenArray tokens) {
	parser.tokens = tokens;
	parser.i = 0;
}

// peek at a token
Token* parser_peek(int offset) {
	// bounds check
	if (parser.i + offset >= (int)parser.tokens.count) {
		printf("parser_peek(): index out of bounds.\n");
		printf("parser.i: %d\n", parser.i);
		printf("offset: %d\n", offset);
		printf("tokens.count: %zu\n", parser.tokens.count);
		return NULL;
	}
	return &parser.tokens.tokens[parser.i + offset];
}

// returns current token and advances
Token* parser_consume() {
	// bounds check
	if (parser.i - 1 >= (int)parser.tokens.count) {
		printf("parser_consume(): index out of bounds.\n");
		return NULL;
	}
	return &parser.tokens.tokens[parser.i++];
}

void node_init(Node* node, Node* parent, NodeType type, Token* token, int children, TypeObj* type_info) {
	node->type = type;
	node->parent = parent;
	node->token = token;
	node->len = 0;
	if (children != 0) {
		node->children = arena_alloc(get_arena(), sizeof(Node) * children);
	}
	node->type_info = type_info;
}

// create expression child node for 'parent'
void parse_expression(Node* parent) {
	Token* token = parser_peek(0);
	if (token->type != TOK_VARIABLE &&
			token->type != TOK_INT_LITERAL &&
			token->type != TOK_ADDRESS &&
			token->type != TOK_POINTER) {
		fprintf(stderr, 
				"Expected variable or literal token. '%d' :%i\n",
				token->type,
				token->line);
		arena_free(get_arena());
		exit(EXIT_FAILURE);  // leaks memory
	}

	// if int literal keep as i64
	TypeObj* type = types_get_type_obj("i64");

	if (token->type == TOK_VARIABLE) {
		type = symbol_table_find_type(token->value);
	}
	else if (token->type == TOK_ADDRESS) {
		parser_consume();
		token = parser_peek(0);

		char temp[TOKEN_BUFFER_SIZE] = {};
		strncpy(temp, symbol_table_find_type(token->value)->name, TOKEN_BUFFER_SIZE);
		strcat(temp, "&");
		type = types_get_type_obj(temp);
	}
	else if (token->type == TOK_POINTER) {
		parser_consume();
		token = parser_peek(0);

		char temp[TOKEN_BUFFER_SIZE] = {};
		strncpy(temp, symbol_table_find_type(token->value)->name, TOKEN_BUFFER_SIZE);
		temp[strlen(temp) - 1] = '\0';
		type = types_get_type_obj(temp);
	}

	if (parser_peek(1)->type == TOK_BRACKET_OPEN) {
		char temp[TOKEN_BUFFER_SIZE] = {};
		strncpy(temp, symbol_table_find_type(token->value)->name, TOKEN_BUFFER_SIZE);
		temp[strlen(temp) - 1] = '\0';
		type = types_get_type_obj(temp);

		// consume variable name and open bracket
		parser_consume();
		parser_consume();

		Node* node_expr = &parent->children[parent->len++];
		node_init(node_expr, parent, NODE_EXPRESSION, token, 1, type);

		Token* token_index = parser_peek(0);
		TypeObj* token_type = types_get_type_obj("i64");
		Node* node_index = &node_expr->children[node_expr->len++];
		node_init(node_index, node_expr, NODE_INDEX, token_index, 0, token_type);

		// consume int literal, close bracket and semicolon
		parser_consume();
		parser_consume();
		parser_consume();

		return;
	}

	Node* node_expr = &parent->children[parent->len++];
	node_init(node_expr, parent, NODE_EXPRESSION, token, 0, type);

	// consume expression token
	parser_consume();

	// consume either semicolon or operator token
	parser_consume();
}

// i is at first expression (expr op expr ;)
void parse_operator(Node* node) {
	// TODO: might be redundant
	/*
	if (token->type != TOK_ADD &&
			token->type != TOK_SUBTRACT &&
			token->type != TOK_MULTIPLY &&
			token->type != TOK_DIVIDE) {
		fprintf(stderr, "Operator not supported '%s'.:%i\n", 
				token->value, token->line);
		arena_free(get_arena());
		exit(EXIT_FAILURE);
	}
	*/

	printf("parse op\n");

	// TODO: fix garbage
	Token* token = parser_peek(1);
	if (token->type == TOK_POINTER || token->type != TOK_ADDRESS)
		token = parser_peek(2);

	// type same as parent
	TypeObj* type = symbol_table_find_type(node->token->value);
	Node* node_op = &node->children[node->len++];
	node_init(node_op, node, NODE_OPERATOR, token, NODE_CHILDREN_COUNT, type);

	// parse_expression will consume semicolons and such
	parse_expression(node_op);
	parse_expression(node_op);
}

// i is at exit token
void parse_exit(Node* parent) {
	if (parser_peek(2)->type != TOK_SEMICOLON) {
		fprintf(stderr, "Expected semicolon after exit.\n");
		arena_free(get_arena());
		exit(EXIT_FAILURE);
	}

	Token* token = parser_peek(0);

	// create exit node
	TypeObj* type = types_get_type_obj("ui32");
	Node* node_exit = &parent->children[parent->len++];
	node_init(node_exit, parent, NODE_STATEMENT, token, NODE_CHILDREN_COUNT, type);

	// move to token after exit
	parser_consume();
	parse_expression(node_exit);
}

void parse_function_call(Node* parent) {
	Token* token = parser_peek(0);
	TypeObj* type = symbol_table_find_type(token->value);

	// create call node
	Node* node_call = &parent->children[parent->len++];
	node_init(node_call, parent, NODE_CALL_FUNC, token, NODE_CHILDREN_COUNT, type);

	// consume function name and open paren
	parser_consume();
	parser_consume();

	// parse for function arguments
	Token* child_token = parser_peek(0);
	while (child_token->type != TOK_PAREN_CLOSE) {
		if (child_token->type == TOK_SEMICOLON) {
			break;
		}

		if (child_token->type != TOK_VARIABLE && child_token->type != TOK_INT_LITERAL) {
			fprintf(stderr, "Expected variable or literal token. %d. :%i\n",
				child_token->type, child_token->line);
			arena_free(get_arena());
			exit(EXIT_FAILURE);
		}

		TypeObj* child_type = types_get_type_obj("NULL");
		if (child_token->type == TOK_VARIABLE) {
			child_type = symbol_table_find_type(child_token->value);
		}

		Node* node = &node_call->children[node_call->len++];
		node_init(node, node_call, NODE_EXPRESSION, child_token, NODE_CHILDREN_COUNT, child_type);

		// consume expression and comma / close paren
		parser_consume();
		parser_consume();

		child_token = parser_peek(0);
	}

	// consume semicolon
	parser_consume();
	if (child_token->type == TOK_PAREN_CLOSE) {
		parser_consume();
	}
}

// takes in variable / declaration node
// parser is at variable name token
void parse_variable_right_side(Node* node) {
	// consume variable name
	parser_consume();

	const Token* token = parser_peek(0);

	if (token->type == TOK_SEMICOLON) {
		parser_consume();
		return;
	}
	if (token->type == TOK_EQUALS) {
		parser_consume();
		token = parser_peek(0);
	}
	if (token->type == TOK_BRACKET_OPEN) {
		parser_consume();
		parser_consume();
		parser_consume();
		parser_consume();
		token = parser_peek(0);
		printf("ABC: %s %d\n", token->value, token->type);
	}

	if (parser_peek(1)->type == TOK_ADD ||
			parser_peek(1)->type == TOK_SUBTRACT ||
			parser_peek(1)->type == TOK_MULTIPLY ||
			parser_peek(1)->type == TOK_DIVIDE ||
			parser_peek(2)->type == TOK_ADD ||
			parser_peek(2)->type == TOK_SUBTRACT ||
			parser_peek(2)->type == TOK_MULTIPLY ||
			parser_peek(2)->type == TOK_DIVIDE) {
		parse_operator(node);
	}
	else if (token->type == TOK_FUNC_NAME) {
		parse_function_call(node);
	}
	else {
		parse_expression(node);
	}
}

// parser is at variable type token
void parse_variable_declaration(Node* parent) {
	// type token
	const Token* token_type = parser_peek(0);
	TypeObj* type = types_get_type_obj(token_type->value);

	// consume type
	parser_consume();
	Token* token_name = parser_peek(0);

	Node* node_stmt = &parent->children[parent->len++];
	node_init(node_stmt, parent, NODE_DECLARATION, token_name, NODE_CHILDREN_COUNT, type);

	if (parent->type != NODE_STRUCT) {
		symbol_table_append(node_stmt);
	}

	if (types_is_struct(type)) {
		const Node* struct_node = symbol_table_find_struct(type);

		for (int i = 0; i < struct_node->len; i++) {
			const Token* member_token = struct_node->children[i].token;
			TypeObj* member_type = struct_node->children[i].type_info;

			char name[TOKEN_BUFFER_SIZE] = {};
			sprintf(name, "%s.%s", token_name->value, member_token->value);

			Token* child_token = arena_alloc(get_arena(), sizeof(Token));
			token_init(name, child_token, &parser.tokens);

			Node* member_node = &parent->children[parent->len++];
			node_init(member_node, parent, NODE_IGNORE, child_token, NODE_CHILDREN_COUNT, member_type);

			symbol_table_append(member_node);
		}

		// consume name and semicolon
		parser_consume();
		parser_consume();
		return;
	}

	parse_variable_right_side(node_stmt);
}

// i is at variable name token or pointer token
void parse_variable(Node* parent) {
	Token* token = parser_peek(0);
	TypeObj* type;

	if (token->type == TOK_POINTER) {
		parser_consume();
		token = parser_peek(0);

		char temp[TOKEN_BUFFER_SIZE] = {};
		strncpy(temp, symbol_table_find_type(token->value)->name, TOKEN_BUFFER_SIZE);
		temp[strlen(temp) - 1] = '\0';
		type = types_get_type_obj(temp);
	}
	else if (parser_peek(1)->type == TOK_BRACKET_OPEN) {
		char temp[TOKEN_BUFFER_SIZE] = {};
		strncpy(temp, symbol_table_find_type(token->value)->name, TOKEN_BUFFER_SIZE);
		temp[strlen(temp) - 1] = '\0';
		type = types_get_type_obj(temp);

		Node* node_stmt = &parent->children[parent->len++];
		node_init(node_stmt, parent, NODE_STATEMENT, token, NODE_CHILDREN_COUNT, type);

		Token* token_index = parser_peek(2);
		TypeObj* token_type = types_get_type_obj("i64");
		Node* node_index = &node_stmt->children[node_stmt->len++];
		node_init(node_index, node_stmt, NODE_INDEX, token_index, 0, token_type);

		parse_variable_right_side(node_stmt);

		return;
	}
	else {
		type = symbol_table_find_type(token->value);
	}

	Node* node_stmt = &parent->children[parent->len++];
	node_init(node_stmt, parent, NODE_STATEMENT, token, NODE_CHILDREN_COUNT, type);

	parse_variable_right_side(node_stmt);
}

// forward declare because is C is dogshit
void parse_node(Node*);

// i is at function type token
void parse_function_declaration(Node* parent) {
	const Token* token_type = parser_peek(0);
	TypeObj* type = types_get_type_obj(token_type->value);

	// consume type token
	parser_consume();
	Token* token_name = parser_peek(0);

	Node* node_func = &parent->children[parent->len++];
	node_init(node_func, parent, NODE_DECLARATION_FUNC, token_name, NODE_ROOT_CHILDREN_COUNT, type);

	symbol_table_append(node_func);

	// consume function name, open paren
	parser_consume();
	parser_consume();

	// parse arguments
	Token* child_token = parser_peek(0);
	while (child_token->type != TOK_PAREN_CLOSE) {
		if (child_token->type == TOK_BRACE_OPEN) {
			break;
		}

		if (child_token->type != TOK_TYPE) {
			fprintf(stderr, "Incorrect argument syntax. Expected type. :%i\n", child_token->line);
			arena_free(get_arena());
			exit(EXIT_FAILURE);
		}
		TypeObj* child_type = types_get_type_obj(child_token->value);

		// consume type token
		parser_consume();

		child_token = parser_peek(0);
		if (child_token->type != TOK_VARIABLE) {
			fprintf(stderr, "Incorrect argument syntax. Expected variable :%i\n", child_token->line);
			arena_free(get_arena());
			exit(EXIT_FAILURE);
		}

		Node* node = &node_func->children[node_func->len++];
		node_init(node, node_func, NODE_LOCAL_VARIABLE, child_token, NODE_CHILDREN_COUNT, child_type);

		symbol_table_append(node);

		// consume variable name and comma
		parser_consume();
		parser_consume();

		child_token = parser_peek(0);
	}

	// consume close paren and open brace
	parser_consume();
	if (child_token->type == TOK_PAREN_CLOSE) {
		parser_consume();
	}

	parse_node(node_func);
}

void parse_return(Node* parent) {
	if (parser_peek(2)->type != TOK_SEMICOLON) {
		fprintf(stderr, "Expected semicolon after return.\n");
		arena_free(get_arena());
		exit(EXIT_FAILURE);
	}

	Token* token = parser_peek(0);

	// return node type is the same as function return type
	TypeObj* type = parent->type_info;

	// create return node
	Node* node_ret = &parent->children[parent->len++];
	node_init(node_ret, parent, NODE_STATEMENT, token, NODE_CHILDREN_COUNT, type);

	parser_consume();
	parse_expression(node_ret);
}

void parse_struct(Node* parent) {
	Token* token_type = parser_peek(1);
	parser_consume();
	parser_consume();

	TypeObj* _type = types_get_type_obj("NULL");
	Node* node_struct = &parent->children[parent->len++];
	node_init(node_struct, parent, NODE_STRUCT, token_type, NODE_ROOT_CHILDREN_COUNT, _type);

	// consume open brace
	parser_consume();

	parse_node(node_struct);

	symbol_table_append(node_struct);
}

void parse_assembly(Node* parent) {
	// consume function name and open paren
	parser_consume();
	parser_consume();

	Token* token = parser_peek(0);
	if (token->type != TOK_STRING_LITERAL) {
		fprintf(stderr, "Expected string as argument for builtin 'asm'. :%i\n",
			token->line);
		arena_free(get_arena());
		exit(EXIT_FAILURE);
	}

	TypeObj* type = types_get_type_obj("NULL");

	Node* node_asm = &parent->children[parent->len++];
	node_init(node_asm, parent, NODE_ASSEMBLY, token, 0, type);

	// consume string literal, close paren and semicolon;
	parser_consume();
	parser_consume();
	parser_consume();
}

// parse for children of one depth (kind of)
void parse_node(Node* parent) {
	Token* token = parser_peek(0);

	while (token != NULL) {
		if (token->type == TOK_BRACE_CLOSE) {
			// this means
			parser_consume();
			return;
		}

		if (token->type == TOK_EXIT) {
			parse_exit(parent);
		}

		else if (token->type == TOK_FUNC_NAME) {
			// builtin asm 'function'
			if (strcmp(BUILTIN_ASSEMBLY_SYMBOL, token->value) == 0) {
				parse_assembly(parent);
			} else {
				parse_function_call(parent);
			}
		}

		else if (token->type == TOK_STRUCT) {
			parse_struct(parent);
		}

		else if (token->type == TOK_TYPE) {
			if (parser_peek(1)->type == TOK_FUNC_NAME) {
				parse_function_declaration(parent);
			}
			else if (parser_peek(1)->type == TOK_VARIABLE) {
				parse_variable_declaration(parent);
			}
			else {
				fprintf(stderr, 
						"Unknown or missing token after type specifier. :%i\n", 
						token->line);
			}
		}

		else if (token->type == TOK_VARIABLE || token->type == TOK_POINTER) {
			parse_variable(parent);
		}

		else if (token->type == TOK_RETURN) {
			parse_return(parent);
		}
		
		else {
			fprintf(stderr, 
					"Invalid token type (ignoring). '%d' :%i\n",
					token->type,
					token->line);
			exit(1);
		}

		token = parser_peek(0);
	}
}

// parse entire program
void parse(Node* root, const TokenArray tokens) {
	parser_init(tokens);

	while (parser_peek(0) != NULL) {
		parse_node(root);
	}
}

void print_tree(const Node* root) {
	printf("\nroot length: %d\n", root->len);
	for (int i = 0; i < root->len; i++) {
		printf("node type: %d, token type: %d, children: %d, :%i\n",
				root->children[i].type,
				root->children[i].token->type,
				root->children[i].len,
				root->children[i].token->line);
	}
}
