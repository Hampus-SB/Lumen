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

	Token token;
} Node;

typedef struct {
	Node children[NODE_ROOT_CHILDREN_COUNT];
	int len;
} NodeRoot;

void parse(NodeRoot* root, Token** tokens, int len) {
	Token* token;
	for (int i = 0; i < len; i++) {
		token = tokens[i];

		if (token->type == EXIT) {
			i++;
			if (i + 1 < len && tokens[i]->type == INT_LITERAL) {
				// create exit node
				Node* exit_node = &root->children[root->len++];
				exit_node->type = STATEMENT;
				exit_node->parent = NULL;
				exit_node->token = *tokens[i];
				exit_node->children = malloc(sizeof(Node) * NODE_CHILDREN_COUNT);
	
				// create expr node (int literal)
				Node* expr_node = &exit_node->children[exit_node->len++];
				expr_node->type = EXPRESSION;
				expr_node->parent = exit_node;
				expr_node->token = *tokens[i + 1];

				continue;
			}
		} 

		else if (token->type == SEMICOLON) {
			printf("Skipping semicolon token\n");
		} 

		else {
			fprintf(stderr, "Invalid token type.\n");
		}
	}
}
