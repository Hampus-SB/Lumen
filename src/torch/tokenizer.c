#include "../../include/torch/types.h"
#include "../../include/torch/tokenizer.h"

#define LUMEN_ARENA_IMPLEMENTATION
#include "../../include/torch/arena.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "logging.h"

// the index need to match the TokenType enum
const char* keywords[] = {
	"=", "+", "-", "*", "/",
	"(", ")", "{", "}", "[", "]",
	",", "&", "*", ";",
	"return", "struct",
};

int line;

// handles escape characters
void string_literal_fix(char* dest, const char* src) {
	int dest_i = 0;
	for (int i = 0; i < strlen(src); i++) {
		if (src[i] == '\\') {
			if (src[i + 1] == 't') {
				dest[dest_i++] = 0x09;  // \t
			} else if (src[i + 1] == 'n') {
				dest[dest_i++] = 0x0a;  // \n
			}
			i++;
		} else {
			dest[dest_i++] = src[i];
		}
	}
}

void token_print(const Token* token) {
	char output[128] = {0};
	strcat(output, "token: ");

	char temp[32] = {0};

	if (token->type < 17) {
		sprintf(temp, "'%s', %d", keywords[token->type], token->type);
		strcat(output, temp);
	} else if (token->type == TOK_TYPE) {
		sprintf(temp, "'type'");
		strcat(output, temp);
	}else if (token->type == TOK_VARIABLE) {
		sprintf(temp, "'variable'");
		strcat(output, temp);
	} else if (token->type == TOK_FUNC_NAME) {
		sprintf(temp, "'func_name'");
		strcat(output, temp);
	} else if (token->type == TOK_INT_LITERAL) {
		sprintf(temp, "'int_lit'");
		strcat(output, temp);
	} else if (token->type == TOK_STRING_LITERAL) {
		sprintf(temp, "'str_lit'");
		strcat(output, temp);
	} else {
		sprintf(temp, "'balls'");
		strcat(output, temp);
	}
	if (token->has_value) {
		sprintf(temp, ", '%s'", token->value);
		strcat(output, temp);
	}
	loginfo("%s", output);
}

void token_append(TokenArray* tokens, const Token* token) {
	if (tokens->count + 1 > tokens->capacity) {
		arena_extend(&arena);
		tokens->capacity *= 2;
	}
	tokens->tokens[tokens->count++] = *token;
}

void token_init(const char* str, Token* token, TokenArray* tokens) {
	token->has_value = 0;  // default to false
	token->line = line;

	if (str[strlen(str) - 1] == '"') {
		token->has_value = 1;
		strncpy(token->value, str, TOKEN_BUFFER_SIZE);
		token->value[strlen(token->value) - 1] = '\0';
		token->type = TOK_STRING_LITERAL;
		char temp[TOKEN_BUFFER_SIZE] = {};
		string_literal_fix(temp, token->value);
		strncpy(token->value, temp, TOKEN_BUFFER_SIZE);
		return;
	}

	if (types_exists(str)) {
		token->has_value = 1;
		strncpy(token->value, str, TOKEN_BUFFER_SIZE);
		token->type = TOK_TYPE;
		return;
	}

	if (tokens->count > 0) {
		if ((tokens->tokens[tokens->count - 1].type == TOK_SEMICOLON ||
				tokens->tokens[tokens->count - 1].type == TOK_EQUALS ||
				tokens->tokens[tokens->count - 1].type == TOK_ADD ||
				tokens->tokens[tokens->count - 1].type == TOK_SUBTRACT ||
				tokens->tokens[tokens->count - 1].type == TOK_MULTIPLY ||
				tokens->tokens[tokens->count - 1].type == TOK_DIVIDE) &&
				strcmp(str, "*") == 0) {
			token->type = TOK_POINTER;
			return;
		}
	}

	for (int i = 0; i < KEYWORD_COUNT; i++) {
		if (strcmp(str, keywords[i]) == 0) {
			token->type = i;
			return;
		}
	}

	// determine if type is a literal or a variable name
	// or function name or struct name
	int int_lit = 0;
	if (isdigit(str[0])) {
		for (int i = 1; i < strlen(str); i++) {
			if (!isdigit(str[i])) {
				logerror("Invalid syntax. '%s' :%i", str, line);
				arena_free(&arena);
				exit(EXIT_FAILURE);
			}
		}
		int_lit = 1;
	}

	token->has_value = 1;

	if (tokens->tokens[tokens->count - 1].type == TOK_STRUCT) {
		token->type = TOK_TYPE;
		strncpy(token->value, str, TYPE_NAME_SIZE);
		types_append(token->value, BITS_NONE);
	}
	else if (int_lit) {
		token->type = TOK_INT_LITERAL;
	}
	else if (str[strlen(str) - 1] == FUNCTION_SPECIAL) {
		token->type = TOK_FUNC_NAME;

		char s[TOKEN_BUFFER_SIZE];
		strcpy(s, str);
		s[strlen(s) - 1] = '\0';

		strcpy(token->value, s);
		return;
	}
	else {
		token->type = TOK_VARIABLE;
	}

	// store the token value as a string
	strcpy(token->value, str);
}

void add_token(TokenArray* tokens, const char* buffer) {
	// if buffer is empty there is no token
	if (buffer[0] == '\0')
		return;

	int offset = 0;
	char* buf = malloc(sizeof(char) * TOKEN_BUFFER_SIZE);
	memset(buf, 0, TOKEN_BUFFER_SIZE);
	strcpy(buf, buffer);
	
	while (isspace(*buf)) { buf++; offset--; };

	Token token;
	token_init(buf, &token, tokens);
	if (tokens->count > 0 &&
			tokens->tokens[tokens->count - 1].type == TOK_TYPE &&
			token.type == TOK_ADDRESS) {
		char temp[TOKEN_BUFFER_SIZE] = {};
		strcpy(temp, tokens->tokens[tokens->count - 1].value);
		strcat(temp, "&");
		token_init(temp, &tokens->tokens[tokens->count - 1], tokens);
	} else {
		token_append(tokens, &token);
	}

	free(buf + offset);
}

// TODO: conjoin the if statements
void tokens_from_source(const char* src, TokenArray* tokens) {
	char buffer[TOKEN_BUFFER_SIZE] = {0};

	int idx_buf = 0;
	int idx_src = 0;
	
	line = 1;

	// replace with for loop
	// TODO: rewrite this dogshit
	while (1) {
		char c = src[idx_src++];

		if (c == '\0') break;  // EOF

		if (c == '\n') {
			line++;
			continue;
		}

		// single-line comments
		if (c == '/' && src[idx_src] == '/') {
			while (c != '\n') {
				c = src[idx_src++];
			}
			line++;
			continue;
		}

		// block comments
		if (c == '/' && src[idx_src] == '*') {
			while (1) {
				if (c == '*' && src[idx_src] == '/')
					break;
				if (c == '\n')
					line++;
				c = src[idx_src++];
			}
			idx_src++;
			continue;
		}

		if (c == '"') {
			c = src[idx_src++];

			while (c != '"') {
				// TODO: bounds checking
				buffer[idx_buf++] = c;
				c = src[idx_src++];
			}

			buffer[idx_buf++] = '"';
			//idx_src++;

			add_token(tokens, buffer);

			memset(buffer, 0, TOKEN_BUFFER_SIZE);
			idx_buf = 0;
		}
		else if (c == ';') {
			// add token thats currently in buffer
			add_token(tokens, buffer);
			memset(buffer, 0, TOKEN_BUFFER_SIZE);
			idx_buf = 0;

			// add the ; token
			add_token(tokens, ";");
		}
		else if (c == '{') {
			// add token thats currently in buffer
			add_token(tokens, buffer);
			memset(buffer, 0, TOKEN_BUFFER_SIZE);
			idx_buf = 0;

			// add the { token
			add_token(tokens, "{");
		}
		else if (c == '}') {
			// add token thats currently in buffer
			add_token(tokens, buffer);
			memset(buffer, 0, TOKEN_BUFFER_SIZE);
			idx_buf = 0;

			// add the } token
			add_token(tokens, "}");
		}
		else if (c == '(') {
			// manipulate buffer
			buffer[idx_buf++] = FUNCTION_SPECIAL;

			// add token thats currently in buffer
			add_token(tokens, buffer);
			memset(buffer, 0, TOKEN_BUFFER_SIZE);
			idx_buf = 0;
			
			// add the ( token
			add_token(tokens, "(");
		}
		else if (c == ')') {
			// add token thats currently in buffer
			add_token(tokens, buffer);
			memset(buffer, 0, TOKEN_BUFFER_SIZE);
			idx_buf = 0;
			
			// add the ) token
			add_token(tokens, ")");
		}
		else if (c == '[') {
			// add token thats currently in buffer
			add_token(tokens, buffer);
			memset(buffer, 0, TOKEN_BUFFER_SIZE);
			idx_buf = 0;

			// add the ( token
			add_token(tokens, "[");
		}
		else if (c == ']') {
			// add token thats currently in buffer
			add_token(tokens, buffer);
			memset(buffer, 0, TOKEN_BUFFER_SIZE);
			idx_buf = 0;

			// add the ) token
			add_token(tokens, "]");
		}
		else if (c == ',') {
			// add token thats currently in buffer
			add_token(tokens, buffer);
			memset(buffer, 0, TOKEN_BUFFER_SIZE);
			idx_buf = 0;

			// add the ) token
			add_token(tokens, ",");
		}
		else if (c == '&') {
			// add token thats currently in buffer
			add_token(tokens, buffer);
			memset(buffer, 0, TOKEN_BUFFER_SIZE);
			idx_buf = 0;

			// add the ) token
			add_token(tokens, "&");
		}
		else if (c == '*') {
			// add token thats currently in buffer
			add_token(tokens, buffer);
			memset(buffer, 0, TOKEN_BUFFER_SIZE);
			idx_buf = 0;

			// add the ) token
			add_token(tokens, "*");
		}
		else if (c == ' ') {
			// add token from buffer
			add_token(tokens, buffer);
			memset(buffer, 0, TOKEN_BUFFER_SIZE);
			idx_buf = 0;
		}
		else {
			buffer[idx_buf++] = c;
		}
	}
}
