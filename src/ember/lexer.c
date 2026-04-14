#include "lexer.h"
#include <ctype.h>
#include <string.h>

int lex_line(const char *line, Token *tokens, int max_tokens) {
    int i = 0;
    int t = 0;

    while (line[i] != '\0' && t < max_tokens) {

        // is whitespace?
        if (isspace(line[i])) {
            i++;
            continue;
        }

        // comma
        if (line[i] == ',') {
            tokens[t].type = TOK_COMMA;
            strcpy(tokens[t].text, ",");
            t++;
            i++;
            continue;
        }

        // string literal
        if (line[i] == '"') {
            i++; // skip opening quote
            int start = i;
            
            // get length of string
            while (line[i] != '"' && line[i] != '\0') i++;

            int len = i - start;
            strncpy(tokens[t].text, &line[start], len); // save string into tokens.text
            tokens[t].text[len] = '\0'; // add nullterminator
            tokens[t].type = TOK_STRING; // assign type string to token
            t++; // increment token counter

            if (line[i] == '"') i++; // skip closing quote
            continue;
        }

        // if memory management , []
        if (line[i] == '[') {
            int start = i;
            
            // get length
            while (line[i] != ']') i++;
            i++;

            int len = i - start;
            strncpy(tokens[t].text, &line[start], len);
            tokens[t].text[len] = '\0'; // add nullterminator
            tokens[t].type = TOK_MEMMAN; // assign type MEMMAN to token
            t++; // increment token counter

            continue;
        }


        // identifier / label / register / number / the shit
        if (isalnum(line[i]) || line[i] == '_' || line[i] == '.') {
            int start = i;

            while (isalnum(line[i]) || line[i] == '_' || line[i] == '.') i++; // same shit

            int len = i - start; // get legnt bro
            strncpy(tokens[t].text, &line[start], len);
            tokens[t].text[len] = '\0';

            // label (ends with :) :)
            if (line[i] == ':') {
                tokens[t].type = TOK_LABEL;
                i++;
            }
            // if register
            else if (tokens[t].text[0] == 'r') {
                tokens[t].type = TOK_REGISTER;
            }
            // number
            else if (isdigit(tokens[t].text[0])) {
                tokens[t].type = TOK_NUMBER;
            }
            else {
                tokens[t].type = TOK_IDENT;
            }

            t++;
            continue;
        }

        // unknown character → skip
        i++;
    }

    return t;
}
