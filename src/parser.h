#include "token.h"

#define NODE_CHILDREN_COUNT 8
#define NODE_ROOT_CHILDREN_COUNT 64

typedef enum {
	NODE_DECLARATION,  // variable declaration
	NODE_STATEMENT,
	NODE_EXPRESSION,
	NODE_OPERATOR,
	NODE_DECLARATION_FUNC,
	NODE_CALL_FUNC,
} NodeType;

typedef enum {
	NO_TYPE,
	I8,
	I16,
	I32,
	I64,
	UI8,
	UI16,
	UI32,
	UI64,
} Type;

typedef struct Node {
	NodeType type;
	struct Node* parent;
	struct Node* children;
	int len;  // how many children
	
	Token* token;
	Type _type;  // NULL if the node does not have an associated type
} Node;

typedef struct {
	TokenArray tokens;
	int i;
} Parser;

Parser parser;

typedef struct {
	char name[TOKEN_BUFFER_SIZE];
	Type type;
} Name;

typedef struct {
	Name names[512];
	int i;
	// int scope;
} NameTracker;

NameTracker names;

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

void names_add(const char* name, Type type) {
	strcpy(names.names[names.i].name, name);
	names.names[names.i].type = type;
	names.i++;
}

// get type of known variable or function
Type names_get_type(const char* name) {
	for (int i = 0; i < names.i; i++) {
		if (strcmp(name, names.names[i].name) == 0) {
			return names.names[i].type;
		}
	}
	fprintf(stderr, "Failed to fetch type of name.\n");
	return NO_TYPE;
}

void node_init(Node* node, Node* parent, NodeType type, Token* token, int children, Type _type) {
	node->type = type;
	node->parent = parent;
	node->token = token;
	node->_type = _type;
	node->len = 0;
	if (children != 0) {
		node->children = arena_alloc(&arena, sizeof(Node) * children);
	}
}

// returns the type of a token
Type type_lookup(const Token* token) {
	if (!token->has_value) {
		fprintf(stderr,  "Error looking up type. Token has no value.\n");
		return NO_TYPE;
	}

	const char* type = token->value;
	if (strcmp(type, "i8") == 0) return I8;
	if (strcmp(type, "i16") == 0) return I16;
	if (strcmp(type, "i32") == 0) return I32;
	if (strcmp(type, "i64") == 0) return I64;
	if (strcmp(type, "ui8") == 0) return UI8;
	if (strcmp(type, "ui16") == 0) return UI16;
	if (strcmp(type, "ui32") == 0) return UI32;
	if (strcmp(type, "ui64") == 0) return UI64;

	fprintf(stderr,  "Error looking up type. '%s'\n", token->value);
	return NO_TYPE;
}

// create expression child node for 'parent'
void parse_expression(Node* parent) {
	Node* node_expr = &parent->children[parent->len++];
	
	Token* token = parser_peek(0);
	if (token->type != VARIABLE && 
			token->type != INT_LITERAL) {
		fprintf(stderr, 
				"Expected variable or literal token. :%i\n", 
				token->line);
		arena_free(&arena);
		exit(EXIT_FAILURE);  // leaks memory
	}

	// if int literal keep as I64
	Type type = I64;
	if (token->type == VARIABLE) {
		type = names_get_type(token->value);
	}

	// NOTE: children are not allocated for the expression node
	node_init(node_expr, parent, NODE_EXPRESSION, token, 0, type);

	// consume expression token
	parser_consume();

	// consume either semicolon or operator token
	parser_consume();
}

// i is at first expression (expr op expr ;)
void parse_operator(Node* node) {
	Token* token = parser_peek(1);

	if (token->type != ADD &&
			token->type != SUBTRACT &&
			token->type != MULTIPLY &&
			token->type != DIVIDE) {
		fprintf(stderr, "Operator not supported '%s'.:%i\n", 
				token->value, token->line);
		arena_free(&arena);
		exit(EXIT_FAILURE);
	}

	Node* node_op = &node->children[0];
	node_init(node_op, node, NODE_OPERATOR, token, NODE_CHILDREN_COUNT, NO_TYPE);

	// parse_expression will consume semicolons and such
	parse_expression(node_op);
	parse_expression(node_op);
}

// i is at exit token
void parse_exit(Node* parent) {
	if (parser_peek(2)->type != SEMICOLON) {
		fprintf(stderr, "Expected semicolon after exit.\n");
		arena_free(&arena);
		exit(EXIT_FAILURE);
	}

	Token* token = parser_peek(0);

	// create exit node
	Node* node_exit = &parent->children[parent->len++];
	node_init(node_exit, parent, NODE_STATEMENT, token, NODE_CHILDREN_COUNT, NO_TYPE);

	// move to token after exit
	parser_consume();
	parse_expression(node_exit);
}

void parse_function_call(Node* parent) {
	if (parser_peek(1)->type != PAREN_OPEN && 
			parser_peek(2)->type != PAREN_CLOSE &&
			parser_peek(3)->type != SEMICOLON) {
		fprintf(stderr, "Invalid syntax for function call.\n");
		arena_free(&arena);
		exit(EXIT_FAILURE);
	}

	Token* token = parser_peek(0);
	const Type type = names_get_type(token->value);

	// create call node
	Node* node_call = &parent->children[parent->len++];
	node_init(node_call, parent, NODE_CALL_FUNC, token, NODE_CHILDREN_COUNT, type);

	parser_consume();
	parser_consume();
	parser_consume();
	parser_consume();
}

// i is at variable type token
void parse_variable_declaration(Node* parent) {
	// type token
	const Token* token_type = parser_peek(0);
	const Type type = type_lookup(token_type);

	// consume type
	parser_consume();
	Token* token_name = parser_peek(0);

	names_add(token_name->value, type);

	Node* node_stmt = &parent->children[parent->len++];
	node_init(node_stmt, parent, NODE_DECLARATION, token_name, NODE_CHILDREN_COUNT, type);

	if (parser_peek(3)->type == SEMICOLON) {
		// consume variable name and equal sign
		parser_consume();
		parser_consume();

		parse_expression(node_stmt);
	}
	else if (parser_peek(3)->type == PAREN_OPEN) {
		// consume variable name and equal sign
		parser_consume();
		parser_consume();

		// parse for function return value
		parse_function_call(node_stmt);
	}
	else if (parser_peek(5)->type == SEMICOLON) {
		// consume variable name and equal sign
		parser_consume();
		parser_consume();

		parse_operator(node_stmt);
	}
	else {
		fprintf(stderr, 
				"Missing tokens after variable or misplaced ';'. :%i\n", 
				parser_peek(0)->line);
		arena_free(&arena);
		exit(EXIT_FAILURE);
	}
}

// i is at variable name token
void parse_variable(Node* parent) {
	Token* token = parser_peek(0);
	const Type type = names_get_type(token->value);

	Node* node_stmt = &parent->children[parent->len++];
	node_init(node_stmt, parent, NODE_STATEMENT, token, NODE_CHILDREN_COUNT, type);

	if (parser_peek(3)->type == SEMICOLON) {
		// consume variable name and equal sign
		parser_consume();
		parser_consume();

		parse_expression(node_stmt);
	}
	else if (parser_peek(3)->type == PAREN_OPEN) {
		// consume variable name and equal sign
		parser_consume();
		parser_consume();

		// parse for function return value
		parse_function_call(node_stmt);
	}
	else if (parser_peek(5)->type == SEMICOLON) {
		// consume variable name and equal sign
		parser_consume();
		parser_consume();

		parse_operator(node_stmt);
	}
	else {
		fprintf(stderr,
				"Missing tokens after variable or misplaced ';'. :%i\n",
				parser_peek(0)->line);
		arena_free(&arena);
		exit(EXIT_FAILURE);
	}
}

// forward declare because is C is dogshit
void parse_node(Node*);

// i is at function type token
void parse_function_declaration(Node* parent) {
	const Token* token_type = parser_peek(0);
	const Type type = type_lookup(token_type);

	// consume type token
	parser_consume();
	Token* token_name = parser_peek(0);

	names_add(token_name->value, type);

	Node* node_func = &parent->children[parent->len++];
	node_init(node_func, parent, NODE_DECLARATION_FUNC, token_name, NODE_ROOT_CHILDREN_COUNT, type);

	// consume function name, parenthesis, open brace tokens
	parser_consume();
	parser_consume();
	parser_consume();
	parser_consume();

	parse_node(node_func);
}

void parse_return(Node* parent) {
	if (parser_peek(2)->type != SEMICOLON) {
		fprintf(stderr, "Expected semicolon after return.\n");
		arena_free(&arena);
		exit(EXIT_FAILURE);
	}

	Token* token = parser_peek(0);

	// create return node
	Node* node_ret = &parent->children[parent->len++];
	node_init(node_ret, parent, NODE_STATEMENT, token, NODE_CHILDREN_COUNT, NO_TYPE);

	parser_consume();
	parse_expression(node_ret);
}

// parse for children of one depth (kindof)
void parse_node(Node* parent) {
	Token* token = parser_peek(0);

	while (token != NULL) {
		if (token->type == BRACE_CLOSE) {
			// this means
			parser_consume();
			return;
		}

		if (token->type == EXIT) {
			parse_exit(parent);
		}

		else if (token->type == FUNC_NAME) {
			parse_function_call(parent);
		}

		else if (token->type == TYPE) {
			if (parser_peek(1)->type == FUNC_NAME) {
				parse_function_declaration(parent);
			}
			else if (parser_peek(1)->type == VARIABLE) {
				parse_variable_declaration(parent);
			}
			else {
				fprintf(stderr, 
						"Unknown or missing token after type specifier. :%i\n", 
						token->line);
			}
		}

		else if (token->type == VARIABLE) {
			parse_variable(parent);
		}

		else if (token->type == RETURN) {
			parse_return(parent);
		}
		
		else {
			fprintf(stderr, 
					"Invalid token type (ignoring). :%i\n", 
					token->line);
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
		printf("node type: %d, token type: %d, :%i\n", 
				root->children[i].type,
				root->children[i].token->type,
				root->children[i].token->line);
	}
}
