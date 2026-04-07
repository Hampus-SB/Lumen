#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// remove
#define TOKEN_COUNT 32
#define TOKEN_BUFFER_SIZE 32

typedef enum {
	VARIABLE    = 0x00,
	EQUALS      = 0x01,
	SEMICOLON   = 0x02,
	TYPE        = 0x03,
	BRACE_OPEN  = 0x04,
	BRACE_CLOSE = 0x05,
	RETURN      = 0x06,
	INT_LITERAL = 0x07,
} TokenType;

typedef struct {
	TokenType type;
	int has_value;  // boolean
	char value[TOKEN_BUFFER_SIZE];
} Token;



void print_token(Token* token) {
	if (token->type == 0x00) printf("variable");
	else if (token->type == 0x01) printf("=");
	else if (token->type == 0x02) printf(";");
	else if (token->type == 0x03) printf("type");
	else if (token->type == 0x04) printf("{");
	else if (token->type == 0x05) printf("}");
	else if (token->type == 0x06) printf("return");
	else if (token->type == 0x07) printf("int literal");
	else printf("Unsupported token type (this should not happen).\n");

	if (token->has_value) {
		printf(", value: %s", token->value);
	}
	printf("\n");
}



void token_init(const char* str, Token* token) {
	token->has_value = 0;  // default to false
	
	// check for keywords / symbols
	if (strcmp(str, "i32") == 0) token->type = TYPE;
	else if (strcmp(str, "=") == 0) token->type = EQUALS;
	else if (strcmp(str, ";") == 0) token->type = SEMICOLON;
	else if (strcmp(str, "{") == 0) token->type = BRACE_OPEN;
	else if (strcmp(str, "}") == 0) token->type = BRACE_CLOSE;
	else if (strcmp(str, "return") == 0) token->type = RETURN;

	// determine if type is a literal or a variable name
	int int_lit = 0;
	if (isdigit(str[0])) {
		for (int i = 1; i < strlen(str); i++) {
			if (!isdigit(str[i])) {
				printf("Invalid syntax. '%s'\n", str);
				exit(EXIT_FAILURE);  // leaks memory
			}
		}
		int_lit = 1;
	}

	if (int_lit)
		token->type = INT_LITERAL;
	else
		token->type = VARIABLE;

	token->has_value = 1;
	// store the token value as a string
	strcpy(token->value, str);  
}

void add_token(Token** tokens, int* len, const char* buffer) {
	// if buffer is empty there is no token
	if (buffer[0] == '\0')
		return;

	Token* token = malloc(sizeof(Token));
	token_init(buffer, token);
	tokens[*len] = token;
	*len += 1;
}

// TODO: conjoin the if statements
int tokens_from_source(const char* src, Token** tokens) {
	char buffer[TOKEN_BUFFER_SIZE];
	memset(buffer, 0, TOKEN_BUFFER_SIZE);
	
	int idx_tok = 0;
	int idx_buf = 0;
	int idx_src = 0;

	// replace with for loop
	while (1) {
		char c = src[idx_src++];

		if (c == '\0') break;  // EOF
		if (c == '\n') continue;  // ignore newline
		
		if (c == ';') {
			// add token thats currently in buffer
			add_token(tokens, &idx_tok, buffer);
			memset(buffer, 0, TOKEN_BUFFER_SIZE);
			idx_buf = 0;

			// add the ; token
			add_token(tokens, &idx_tok, ";");
		}
		else if (c == '{') {
			// add token thats currently in buffer
			add_token(tokens, &idx_tok, buffer);
			memset(buffer, 0, TOKEN_BUFFER_SIZE);
			idx_buf = 0;

			// add the { token
			add_token(tokens, &idx_tok, "{");
		}
		else if (c == '}') {
			// add token thats currently in buffer
			add_token(tokens, &idx_tok, buffer);
			memset(buffer, 0, TOKEN_BUFFER_SIZE);
			idx_buf = 0;

			// add the } token
			add_token(tokens, &idx_tok, "}");
		}
		else if (c == ' ') {
			// add token from buffer
			add_token(tokens, &idx_tok, buffer);
			memset(buffer, 0, TOKEN_BUFFER_SIZE);
			idx_buf = 0;
		}
		else {
			buffer[idx_buf++] = c;
		}
	}

	return idx_tok;
}
