#include "parser.h"

int validate_compare_nodes(const Node* a, const Node* b) {
    if (b->token->type == TOK_INT_LITERAL) {
        return 1;
    }

    const int result = a->type_info->id == b->type_info->id;
    if (result != 1) {
        fprintf(stderr, "Invalid types. '%s', '%s'. :%i\n",
            a->type_info->name, b->type_info->name,
            a->token->line);
        fprintf(stderr, "WEIRD: '%s', '%s'\n",
            a->token->value, b->token->value);
    }
    return result;
}

int validate_node(const Node* node) {
    switch (node->type) {
        case NODE_DECLARATION:
            if (node->len == 0) {
                return 1;
            }
            // check if both sides are of the same type
            return validate_compare_nodes(node, &node->children[0]);
        case NODE_DECLARATION_FUNC:
            for (int i = 0; i < node->len; i++) {
                if (!validate_node(&node->children[i])) {
                    return 0;
                }
            }
            return 1;
        case NODE_STATEMENT:
            if (node->children[0].type == NODE_OPERATOR) {
                return validate_node(&node->children[0]);
            }
            return validate_compare_nodes(node, &node->children[0]);
        case NODE_OPERATOR:
            const int a = validate_compare_nodes(&node->children[0], &node->children[1]);
            const int b = validate_compare_nodes(node, &node->children[0]);
            return a == 1 && b == 1;
        default:
            break;
    }

    return 1;
}

// returns 1 upon success
int validate_types(const Node* root) {
    for (int i = 0; i < root->len; i++) {
        if (!validate_node(&root->children[i])) {
            return 0;
        }
    }

    return 1;
}
