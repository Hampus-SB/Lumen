#include "../../include/torch/comments.h"

void comments_remove(const char* src, const int src_len, char* output, int* out_len) {
    if (src_len < 1) {
        return;
    }

    *out_len = 0;

    for (int i = 0; i < src_len; i++) {
        if (src[i] == '\0') {
            // EOF
            break;
        }

        char c = src[i];

        // single-line comments
        if (c == '/' && src[i + 1] == '/') {
            while (c != '\n') {
                c = src[++i];
            }
            i--;
            output[*out_len] = '\n';
            *out_len += 1;
            continue;
        }

        // block comments
        if (c == '/' && src[i + 1] == '*') {
            while (1) {
                if (c == '*' && src[i + 1] == '/')
                    break;
                c = src[++i];
            }
            i++;
            continue;
        }

        output[*out_len] = c;
        *out_len += 1;
    }
}
