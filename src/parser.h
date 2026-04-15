#define NODE_CHILDREN_COUNT 8
#define NODE_ROOT_CHILDREN_COUNT 64

#include "token.h"

typedef enum {
	DECLARATION,
	STATEMENT,
	EXPRESSION,
	OPERATOR,
	DECLARATION_FUNC,
} NodeType;

typedef struct Node {
	NodeType type;  // ???
	struct Node* parent;
	struct Node* children;
	int len;  // how many children
	
	Token* token;
} Node;

// parse for 'node' child expression
// create expression child node for 'node'
void parse_expression(Node* node, Token** tokens, int i) {
	Node* expr_node = &node->children[node->len++];
	
	Token* token = tokens[i];
	if (token->type != VARIABLE && token->type != INT_LITERAL) {
		fprintf(stderr, 
				"Expected variable or literal token. :%i\n", 
				token->line);
		exit(EXIT_FAILURE);  // leaks memory
	}

	// NOTE: children are not allocated for the expression node
	expr_node->type = EXPRESSION;
	expr_node->parent = node;
	expr_node->token = token;
	expr_node->len = 0;

	/*
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
void parse_operator(Node* node, Token** tokens, int i) {
	Node* node_op = &node->children[0];
	node_op->type = OPERATOR;
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
}

void parse_exit(Node* node, Token** tokens, int i) {
	Token* token = tokens[i];

	// create exit node
	Node* exit_node = &node->children[node->len++];
	exit_node->type = STATEMENT;
	exit_node->parent = node;
	exit_node->token = token;
	exit_node->len = 0;
	exit_node->children = malloc(sizeof(Node) * 
			NODE_CHILDREN_COUNT);

	// move to token after exit
	parse_expression(exit_node, tokens, i + 1);

	printf("parse_exit\n");
}

// i is at variable name token
void parse_variable(Node* parent, Token** tokens, int* i, int len) {
	Node* node_stmt = &parent->children[parent->len++];
	node_stmt->type = DECLARATION;
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
}

// forward declare because is C is dogshit
void parse_node(Node*, Token**, int*, int);

// i is at function name token
void parse_function(Node* parent, Token** tokens, int* i, int len) {
	Node* node_func = &parent->children[parent->len++];
	node_func->type = DECLARATION_FUNC;
	node_func->parent = parent;
	node_func->len = 0;
	node_func->children = malloc(sizeof(Node) * NODE_CHILDREN_COUNT);
	node_func->token = tokens[*i];

	*i += 4;
	parse_node(node_func, tokens, i, len);
}

// parse for children of one depth (kindof)
void parse_node(Node* parent, Token** tokens, int* index, int len) {
	int i;

	for (i = *index; i < len; i++) {
		Token* token = tokens[i];

		if (parent->type == DECLARATION_FUNC) {
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
			parent->children[parent->len - 1].type = STATEMENT;
		}

		else {
			fprintf(stderr, 
					"Invalid token type (ignoring). :%i\n", 
					token->line);
		}
	}

	// update index for callee
	*index = i;
}

// parse entire program
void parse(Node* root, Token** tokens, int len) {
	for (int i = 0; i < len; i++) {
		parse_node(root, tokens, &i, len);
	}
	return;

	/*
	for (int i = 0; i < len; i++) {
		Token* token = tokens[i];

		if (token->type == EXIT) {
			if (i + 2 < len) {
				parse_exit(root, tokens, i);
				i += 2;
			} else {	
				fprintf(stderr, 
						"Missing tokens after statement. :%i\n",
						token->line);
			}
		}

		else if (token->type == TYPE) {
			if (i + 1 < len && 
					tokens[i + 1]->type == FUNC_NAME) {
				if (!(i + 3 < len && 
							tokens[i + 2]->type == PAREN_OPEN  &&
							tokens[i + 3]->type == PAREN_CLOSE)) {
					fprintf(stderr, 
							"Expected parenthesis after function name. :%i\n", token->line);
				}

				if (i + 5 < len) {
					i += 4;
					parse_function(root, tokens, &i);
				} else {
					fprintf(stderr, 
							"Expected braces after function. :%i\n", 
							token->line);
				}

				continue;
			}
			if (i + 4 < len) {
				// represents the variable
				Node* node_stmt = &root->children[root->len++];
				node_stmt->type = DECLARATION;
				node_stmt->parent = NULL;
				node_stmt->len = 0;
				node_stmt->children = malloc(sizeof(Node) * 
						NODE_CHILDREN_COUNT);
				
				// fetch token after type specifier (variable name)
				node_stmt->token = tokens[++i];  

				if (tokens[i + 3]->type == SEMICOLON) {
					// expression
					i += 2;
					parse_expression(node_stmt, tokens, i);
					i++;  // skip semicolon
				} 
				else if (i + 5 < len && 
						tokens[i + 5]->type == SEMICOLON) {
					// operator
					i += 2;
					parse_operator(node_stmt, tokens, i);
					i += 3;
				} 
				else {
					fprintf(stderr, "Error. %i\n", token->line);
				}
			} else {
				fprintf(stderr, 
						"Not enough tokens for declaration. :%i\n", 
						token->line);
			}
		}

		else if (token->type == VARIABLE) {
			if (i + 3 < len) {
				// parse for operator or expression
				Node* node_stmt = &root->children[root->len++];
				node_stmt->type = STATEMENT;
				node_stmt->parent = NULL;
				node_stmt->len = 0;
				node_stmt->children = malloc(sizeof(Node) * 
						NODE_CHILDREN_COUNT);

				node_stmt->token = tokens[i];

				if (tokens[i + 3]->type == SEMICOLON) {
					// expression
					i += 2;
					parse_expression(node_stmt, tokens, i);
					i++;  // skip semicolon
				} 
				else if (i + 5 < len && 
						tokens[i + 5]->type == SEMICOLON) {
					// operator
					i += 2;
					parse_operator(node_stmt, tokens, i);
					i += 3;
				} 
				else {
					fprintf(stderr, "Error. %i\n", token->line);
				}
			} else {
				fprintf(stderr, "Not enough tokens. :%i\n",
						token->line);
			}
		}

		else if (token->type == SEMICOLON) {
			printf("Skipping semicolon token.\n");
		} 

		else {
			fprintf(stderr, "Invalid token type. :%i\n", token->line);
		}
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
