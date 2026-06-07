#include "../../include/torch/imports.h"

#include "logging.h"

#include <string.h>

const char* source;
int source_length;
int i;

int find_import() {
    int temp_i = i;
    if (source[temp_i++] == 'i' &&
            source[temp_i++] == 'm' &&
            source[temp_i++] == 'p' &&
            source[temp_i++] == 'o' &&
            source[temp_i++] == 'r' &&
            source[temp_i++] == 't' &&
            source[temp_i] == ' ') {
        i += 7;
        return 1;
    }
    return 0;
}

// i is at first character in import filename
void get_import_filename(char* name) {
    while (source[i++] != '\n') {
        strncat(name, &source[i - 1], 1);
    }
    i--;
}

void import(const char* src, const int src_len, char* out, int* out_len) {
    if (src_len < 1) {
        return;
    }

    *out_len = 0;

    source = src;
    source_length = src_len;

    for (i = 0; i < src_len; i++) {
        if (source[i] == '\0') {
            // EOF
            break;
        }

        if (find_import()) {
            char filename[IMPORT_FILENAME_SIZE] = {0};
            get_import_filename(filename);

            char filepath[IMPORT_FILENAME_SIZE] = {0};
            sprintf(filepath, "/home/hampus/code/OS/Lumen/std/%s", filename);

            char file[8192] = {0};
            FILE* fh = fopen(filepath, "r");
            if (!fh) {
                logerror("Failed to open import file. '%s'", filepath);
            }

            char buffer[8192];
            while (fgets(buffer, 8192, fh) != NULL) {
                strcat(file, buffer);
            }

            strcat(out, file);
            *out_len += strlen(file);

            loginfo("Importing file '%s'.", filepath);
        }
        else {
            out[*out_len] = source[i];
            *out_len += 1;
        }
    }
}
