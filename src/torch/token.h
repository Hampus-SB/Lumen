#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define LUMEN_ARENA_IMPLEMENTATION
#include "arena.h"

#define TOKEN_DEFAULT_COUNT 256
#define TOKEN_BUFFER_SIZE 32

#define FUNCTION_SPECIAL '#'

#define MAX_STRUCT_COUNT 32

// the index need to match the TokenType enum
const char* keywords[] = {
	"=", "+", "-", "*", "/",
	"(", ")", "{", "}", ".",
	";", "return", "exit", "struct",
	"i64", "i32", "i16", "i8",
	"ui64", "ui32", "ui16", "ui8",
};

typedef enum {
	TOK_EQUALS,
	TOK_ADD,
	TOK_SUBTRACT,
	TOK_MULTIPLY,
	TOK_DIVIDE,
	TOK_PAREN_OPEN,
	TOK_PAREN_CLOSE,
	TOK_BRACE_OPEN,
	TOK_BRACE_CLOSE,
	TOK_PERIOD,
	TOK_SEMICOLON,
	TOK_RETURN,
	TOK_EXIT,
	TOK_STRUCT,
	TOK_TYPE,

	TOK_VARIABLE,
	TOK_FUNC_NAME,
	TOK_INT_LITERAL,
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

typedef struct {
	char names[MAX_STRUCT_COUNT][TOKEN_BUFFER_SIZE];
	int i;
} Structs;

Structs structs;

void add_struct(const char* name) {
	if (structs.i >= MAX_STRUCT_COUNT) {
		fprintf(stderr, "Too many struct declarations.\n");
		return;
	}
	strcpy(structs.names[structs.i++], name);
}

void token_print(const Token* token) {
	printf("token: ");
	if (token->type < 14) {
		printf("'%s'", keywords[token->type]);
	} else if (token->type == TOK_TYPE) {
		printf("'type'");
	}else if (token->type == TOK_VARIABLE) {
		printf("'variable'");
	} else if (token->type == TOK_FUNC_NAME) {
		printf("'func_name'");
	} else if (token->type == TOK_INT_LITERAL) {
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

void token_init(const char* str, Token* token, TokenArray* tokens) {
	token->has_value = 0;  // default to false
	token->line = line;

	for (int i = 0; i < structs.i; i++) {
		if (strcmp(structs.names[i], str) == 0) {
			token->has_value = 1;
			strncpy(token->value, structs.names[i], TOKEN_BUFFER_SIZE);
			token->type = TOK_TYPE;
			return;
		}
	}

	for (int i = 0; i < 22; i++) {
		if (strcmp(str, keywords[i]) == 0) {
			// matches against a type
			if (i >= 14) {
				token->has_value = 1;
				strncpy(token->value, keywords[i], TOKEN_BUFFER_SIZE);
				token->type = TOK_TYPE;
				return;
			}
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
				printf("Invalid syntax. '%s' :%i\n", str, line);
				arena_free(&arena);
				exit(EXIT_FAILURE);  // leaks memory
			}
		}
		int_lit = 1;
	}

	token->has_value = 1;

	if (tokens->tokens[tokens->count - 1].type == TOK_STRUCT) {
		token->type = TOK_TYPE;
		add_struct(str);
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
		/*
		else if (c == '.') {
			// add token thats currently in buffer
			add_token(tokens, buffer);
			memset(buffer, 0, TOKEN_BUFFER_SIZE);
			idx_buf = 0;

			// add the ) token
			add_token(tokens, ".");
		}
		*/
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
