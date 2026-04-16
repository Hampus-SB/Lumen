#include "token.h"

#define NODE_CHILDREN_COUNT 8
#define NODE_ROOT_CHILDREN_COUNT 64

typedef enum {
	NODE_DECLARATION,
	NODE_STATEMENT,
	NODE_EXPRESSION,
	NODE_OPERATOR,
	NODE_DECLARATION_FUNC,
} NodeType;

// TODO: dynamic arrays
// TODO: dont store token as pointer

typedef struct Node {
	NodeType type;  // ???
	struct Node* parent;
	struct Node* children;
	int len;  // how many children
	
	Token* token;
} Node;

typedef struct {
	Token** tokens;
	int len;
	int i;
} Parser;

Parser parser;

void parser_init(Token** tokens, int len) {
	parser.tokens = tokens;
	parser.len = len;
	parser.i = 0;
}

// peek at a token
Token* parser_peek(int offset) {
	// bounds check
	if (parser.i + offset - 1 >= parser.len) {
		printf("Index out of bounds.\n");
		return NULL;
	}
	return parser.tokens[parser.i + offset];
}

// returns current token and advances
Token* parser_consume() {
	// bounds check
	if (parser.i - 1 >= parser.len) {
		printf("Index out of bounds.\n");
		return NULL;
	}
	return parser.tokens[parser.i++];
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
		exit(EXIT_FAILURE);  // leaks memory
	}

	// NOTE: children are not allocated for the expression node
	node_expr->type = NODE_EXPRESSION;
	node_expr->parent = parent;
	node_expr->token = token;
	node_expr->len = 0;

	// consume expression token
	parser_consume();

	// consume either semicolon or operator token
	parser_consume();



	/*
	Token* token = tokens[i];
	if (token->type != VARIABLE && token->type != INT_LITERAL) {
		fprintf(stderr, 
				"Expected variable or literal token. :%i\n", 
				token->line);
		exit(EXIT_FAILURE);  // leaks memory
	}

	// NOTE: children are not allocated for the expression node
	expr_node->type = NODE_EXPRESSION;
	expr_node->parent = node;
	expr_node->token = token;
	expr_node->len = 0;
	
	*i += 1;
	token = tokens[*i];
	if (token->type != SEMICOLON) {
		fprintf(stderr, "Expected semicolon after expression. :i\n",
				token->line);
		exit(EXIT_FAILURE);
	}
	*/
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
		exit(EXIT_FAILURE);
	}

	Node* node_op = &node->children[0];
	node_op->type = NODE_OPERATOR;
	node_op->parent = node;
	node_op->token = token;
	node_op->len = 0;
	node_op->children = malloc(sizeof(Node) * NODE_CHILDREN_COUNT);

	// parse_expression will consume semicolons and such
	parse_expression(node_op);
	parse_expression(node_op);



	/*
	Node* node_op = &node->children[0];
	node_op->type = NODE_OPERATOR;
	node_op->parent = node;
	node_op->token = tokens[i + 1];
	node_op->len = 0;
	node_op->children = malloc(sizeof(Node) * NODE_CHILDREN_COUNT);

	if (node_op->token->type != ADD &&
			node_op->token->type != SUBTRACT &&
			node_op->token->type != MULTIPLY &&
			node_op->token->type != DIVIDE) {
		fprintf(stderr, "Operator not supported '%s'.:%i\n", 
				node_op->token->value, node_op->token->line);
		exit(EXIT_FAILURE);
	}

	parse_expression(node_op, tokens, i);
	parse_expression(node_op, tokens, i + 2);
	*/
}

// i is at exit token
void parse_exit(Node* parent) {
	if (parser_peek(2)->type != SEMICOLON) {
		fprintf(stderr, "Expected semicolon after exit.\n");
		exit(EXIT_FAILURE);
	}

	Token* token = parser_peek(0);
	
	// create exit node
	Node* node_exit = &parent->children[parent->len++];
	node_exit->type = NODE_STATEMENT;
	node_exit->parent = parent;
	node_exit->token = token;
	node_exit->len = 0;
	node_exit->children = malloc(sizeof(Node) * 
			NODE_CHILDREN_COUNT);

	// move to token after exit
	parser_consume();
	parse_expression(node_exit);



	/*
	Token* token = tokens[i];

	// create exit node
	Node* exit_node = &node->children[node->len++];
	exit_node->type = NODE_STATEMENT;
	exit_node->parent = node;
	exit_node->token = token;
	exit_node->len = 0;
	exit_node->children = malloc(sizeof(Node) * 
			NODE_CHILDREN_COUNT);

	// move to token after exit
	parse_expression(exit_node, tokens, i + 1);

	printf("parse_exit\n");
	*/
}

// i is at variable name token
void parse_variable(Node* parent) {
	Node* node_stmt = &parent->children[parent->len++];
	node_stmt->type = NODE_DECLARATION;
	node_stmt->parent = parent;
	node_stmt->token = parser_peek(0);
	node_stmt->len = 0;
	node_stmt->children = malloc(sizeof(Node) * 
			NODE_CHILDREN_COUNT);

	if (parser_peek(3)->type == SEMICOLON) {
		// consume variable name and equal sign
		parser_consume();
		parser_consume();

		parse_expression(node_stmt);
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
		exit(EXIT_FAILURE);
	}



	/*
	Node* node_stmt = &parent->children[parent->len++];
	node_stmt->type = NODE_DECLARATION;
	node_stmt->parent = parent;
	node_stmt->token = tokens[*i];
	node_stmt->len = 0;
	node_stmt->children = malloc(sizeof(Node) * 
			NODE_CHILDREN_COUNT);

	// i32 name = 1;
	// i32 name = 1 + 2;

	if (*i + 3 >= len) {
		fprintf(stderr, "Missing tokens. :%i\n", 
				tokens[*i]->line);
		return;
	}

	if (tokens[*i + 3]->type == SEMICOLON) {
		// parse for expression
		*i += 2;
		parse_expression(node_stmt, tokens, *i);
		*i += 1;  // skip semicolon
	} 
	else {
		if (*i + 5 >= len) {
			fprintf(stderr, "Missing tokens. :%i\n", 
					tokens[*i]->line);
			return;
		}
		if (tokens[*i + 5]->type == SEMICOLON) {
			*i += 2;
			parse_operator(node_stmt, tokens, *i);
			*i += 3;
		}
	}
	*/
}

// forward declare because is C is dogshit
void parse_node(Node*);

// i is at function name token
void parse_function(Node* parent) {
	Node* node_func = &parent->children[parent->len++];
	node_func->type = NODE_DECLARATION_FUNC;
	node_func->parent = parent;
	node_func->len = 0;
	node_func->children = malloc(sizeof(Node) * NODE_CHILDREN_COUNT);
	node_func->token = parser_peek(0);

	// consume function name, parenthesis, open brace tokens
	parser_consume();
	parser_consume();
	parser_consume();
	parser_consume();

	parse_node(node_func);

	

	/*
	Node* node_func = &parent->children[parent->len++];
	node_func->type = NODE_DECLARATION_FUNC;
	node_func->parent = parent;
	node_func->len = 0;
	node_func->children = malloc(sizeof(Node) * NODE_CHILDREN_COUNT);
	node_func->token = tokens[*i];

	*i += 4;
	parse_node(node_func, tokens, i, len);
	*/
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

		else if (token->type == TYPE) {
			if (parser_peek(1)->type == FUNC_NAME) {
				// consume type token
				parser_consume();
				parse_function(parent);
			}
			else if (parser_peek(1)->type == VARIABLE) {
				// consume type token
				parser_consume();
				parse_variable(parent);
			}
			else {
				fprintf(stderr, 
						"Unknown or missing token after type specifier. :%i\n", 
						token->line);
			}
		}

		else if (token->type == VARIABLE) {
			parse_variable(parent);
			parent->children[parent->len - 1].type = NODE_STATEMENT;
		}
		
		else {
			fprintf(stderr, 
					"Invalid token type (ignoring). :%i\n", 
					token->line);
		}

		token = parser_peek(0);
	}



	/*
	int i;

	for (i = *index; i < len; i++) {
		Token* token = tokens[i];

		if (parent->type == NODE_DECLARATION_FUNC) {
			printf(":%i\n", parent->len);
			printf(":|%i '%s':\n", token->type, token->value);
		}

		if (token->type == BRACE_CLOSE) {
			// exit
			printf("brace close\n");
			*index = i;
			return;
		}

		if (token->type == EXIT) {
			if (i + 2 >= len) {
				fprintf(stderr, 
						"Missing tokens after statement. :%i\n", 
						token->line);
				continue;
			}

			parse_exit(parent, tokens, i);
			i += 2;
		}

		else if (token->type == TYPE) {
			if (i + 4 >= len) {
				fprintf(stderr, 
						"Missing tokens after statement. :%i\n", 
						token->line);
				continue;
			}

			if (tokens[i + 1]->type == FUNC_NAME) {
				// move to function name token
				i++;
				printf("Warning: functions not supported. :%i\n", token->line);
				parse_function(parent, tokens, &i, len);
			} 
			else if (tokens[i + 1]->type == VARIABLE) {
				// move to variable name token
				i++;
				parse_variable(parent, tokens, &i, len);
			} 
			else {
				fprintf(stderr, 
						"Unknown token after type specifier. :%i\n", 
						token->line);
			}
		}

		else if (token->type == VARIABLE) {
			// x = 1;
			// x = 1 + 2;
			parse_variable(parent, tokens, &i, len);
			parent->children[parent->len - 1].type = NODE_STATEMENT;
		}

		else {
			fprintf(stderr, 
					"Invalid token type (ignoring). :%i\n", 
					token->line);
		}
	}

	// update index for callee
	*index = i;
	*/
}

// parse entire program
void parse(Node* root, Token** tokens, int len) {
	parser_init(tokens, len);

	while (parser_peek(0) != NULL) {
		parse_node(root);
	}

	/*
	for (int i = 0; i < len; i++) {
		parse_node(root, tokens, &i, len);
	}
	*/
}

void free_tree(Node* root) {
}

void print_tree(Node* root) {
	for (int i = 0; i < root->len; i++) {
		printf("node type: %d, token type: %d, :%i\n", 
				root->children[i].type,
				root->children[i].token->type,
				root->children[i].token->line);
	}
}
