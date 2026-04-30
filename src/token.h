#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define LUMEN_ARENA_IMPLEMENTATION
#include "arena.h"

#define TOKEN_DEFAULT_COUNT 256
#define TOKEN_BUFFER_SIZE 32

#define FUNCTION_SPECIAL '#'

// the index need to match the TokenType enum
const char* keywords[] = {
	"=", "+", "-", "*", "/",
	"(", ")", "{", "}",
	";", "return", "exit",
	"i64", "i32", "i16", "i8",
	"ui64", "ui32", "ui16", "ui8",
};

typedef enum {
	EQUALS,
	ADD,
	SUBTRACT,
	MULTIPLY,
	DIVIDE,
	PAREN_OPEN,
	PAREN_CLOSE,
	BRACE_OPEN,
	BRACE_CLOSE,
	SEMICOLON,
	RETURN,
	EXIT,
	TYPE,

	VARIABLE,
	FUNC_NAME,
	INT_LITERAL,
} TokenType;

typedef struct {
	TokenType type;
	int has_value;  // boolean
	char value[TOKEN_BUFFER_SIZE];
	int line;  // line number where token is defined in source
} Token;

typedef struct {
	Token* tokens;
	size_t capacity;
	size_t count;
} TokenArray;

int line;

void token_print(const Token* token) {
	printf("token: ");
	if (token->type < 12) {
		printf("'%s'", keywords[token->type]);
	} else if (token->type == TYPE) {
		printf("'type'");
	}else if (token->type == VARIABLE) {
		printf("'variable'");
	} else if (token->type == FUNC_NAME) {
		printf("'func_name'");
	} else if (token->type == INT_LITERAL) {
		printf("'int_lit'");
	} else {
		printf("'balls'");
	}
	if (token->has_value) {
		printf(", '%s'", token->value);
	}
	printf("\n");
}

void token_append(TokenArray* tokens, const Token* token) {
	if (tokens->count + 1 > tokens->capacity) {
		arena_extend(&arena);
		tokens->capacity *= 2;
	}
	tokens->tokens[tokens->count++] = *token;
}

void token_init(const char* str, Token* token) {
	token->has_value = 0;  // default to false
	token->line = line;

	for (int i = 0; i < 20; i++) {
		if (strcmp(str, keywords[i]) == 0) {
			// matches against a type
			if (i >= 12) {
				token->has_value = 1;
				strncpy(token->value, keywords[i], TOKEN_BUFFER_SIZE);
				token->type = TYPE;
				return;
			}
			token->type = i;
			return;
		}
	}

	// determine if type is a literal or a variable name
	// or function name
	int int_lit = 0;
	if (isdigit(str[0])) {
		for (int i = 1; i < strlen(str); i++) {
			if (!isdigit(str[i])) {
				printf("Invalid syntax. '%s' :%i\n", str, line);
				arena_free(&arena);
				exit(EXIT_FAILURE);  // leaks memory
			}
		}
		int_lit = 1;
	}

	token->has_value = 1;

	if (int_lit) {
		token->type = INT_LITERAL;
	} else {
		if (str[strlen(str) - 1] == FUNCTION_SPECIAL) {
			token->type = FUNC_NAME;

			char s[TOKEN_BUFFER_SIZE];
			strcpy(s, str);
			s[strlen(s) - 1] = '\0';

			strcpy(token->value, s);
			return;
		}
		token->type = VARIABLE;
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
	token_init(buf, &token);
	token_append(tokens, &token);

	free(buf + offset);
}

// TODO: conjoin the if statements
void tokens_from_source(const char* src, TokenArray* tokens) {
	char buffer[TOKEN_BUFFER_SIZE] = {0};

	int idx_buf = 0;
	int idx_src = 0;
	
	line = 1;

	// replace with for loop
	while (1) {
		char c = src[idx_src++];

		if (c == '\0') break;  // EOF
		if (c == '\n') { line++; continue; }  // ignore newline
		
		if (c == ';') {
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
