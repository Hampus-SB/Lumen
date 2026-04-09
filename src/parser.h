#define NODE_CHILDREN_COUNT 8
#define NODE_ROOT_CHILDREN_COUNT 64

#include "token.h"

typedef enum {
	STATEMENT,
	EXPRESSION,
} NodeType;

typedef struct Node {
	NodeType type;  // ???
	struct Node* parent;
	struct Node* children;
	int len;  // how many children
	
	Token* token;
} Node;

typedef struct {
	Node children[NODE_ROOT_CHILDREN_COUNT];
	int len;
} NodeRoot;

// parse for 'node' child expression
// create expression childe node for 'node'
void parse_expression(Node* node, Token** tokens, int* i) {
	Node* expr_node = &node->children[node->len++];
	
	Token* token = tokens[*i];
	if (token->type != VARIABLE && token->type != INT_LITERAL) {
		fprintf(stderr, "Expected variable or literal token.\n");
		exit(EXIT_FAILURE);  // leaks memory
	}

	// NOTE: children are not allocated for the expression node
	expr_node->type = EXPRESSION;
	expr_node->parent = node;
	expr_node->token = token;
	expr_node->len = 0;

	*i += 1;
	token = tokens[*i];
	if (token->type != SEMICOLON) {
		fprintf(stderr, "Expected semicolon after expression.\n");
		exit(EXIT_FAILURE);
	}
}

// parse entire program
void parse(NodeRoot* root, Token** tokens, int len) {
	Token* token;
	for (int i = 0; i < len; i++) {
		token = tokens[i];

		if (token->type == EXIT) {
			if (i + 2 < len) {
				// create exit node
				Node* exit_node = &root->children[root->len++];
				exit_node->type = STATEMENT;
				exit_node->parent = NULL;
				exit_node->token = token;
				exit_node->len = 0;
				exit_node->children = malloc(sizeof(Node) * NODE_CHILDREN_COUNT);
	
				i++;  // move to token after exit
				parse_expression(exit_node, tokens, &i);
				continue;
			} else {
				fprintf(stderr, "Missing tokens after statement.\n");
			}
		}

		else if (token->type == TYPE) {
			if (i + 4 < len) {
				// represents the variable
				Node* node_stmt = &root->children[root->len++];
				node_stmt->type = STATEMENT;
				node_stmt->parent = NULL;
				node_stmt->len = 0;
				node_stmt->children = malloc(sizeof(Node) * NODE_CHILDREN_COUNT);
				
				// fetch token after type specifier (variable name)
				node_stmt->token = tokens[++i];  

				// skip equal sign token
				i += 2;

				parse_expression(node_stmt, tokens, &i);
			} else {
				fprintf(stderr, "Not enough tokens for variable declaration.\n");
			}
		}

		else if (token->type == SEMICOLON) {
			printf("Skipping semicolon token.\n");
		} 

		else {
			fprintf(stderr, "Invalid token type.\n");
		}
	}
}

void free_tree(NodeRoot* root) {
}

void print_tree(NodeRoot* root) {
	for (int i = 0; i < root->len; i++) {
		printf("node type: %d, token type: %d\n", 
				root->children[i].type,
				root->children[i].token->type);
	}
}
