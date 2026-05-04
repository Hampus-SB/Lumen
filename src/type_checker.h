#include "parser.h"

int validate_compare_nodes(const Node* a, const Node* b) {
    if (b->token->type == INT_LITERAL) {
        return 1;
    }
    const int result = a->_type == b->_type;
    if (result != 1) {
        fprintf(stderr, "Invalid types. '%d', '%d'\n", a->_type, b->_type);
    }
    return result;
}

int validate_node(const Node* node) {
    switch (node->type) {
        case NODE_DECLARATION:
            // check if both sides are of the same type
            return validate_compare_nodes(node, &node->children[0]);
        case NODE_DECLARATION_FUNC:
            for (int i = 0; i < node->len; i++) {
                if (!validate_node(&node->children[i])) {
                    return 0;
                }
            }
            return 1;
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
