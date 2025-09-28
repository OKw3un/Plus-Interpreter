// lexer.h
#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

// Enum to represent different types of tokens
typedef enum
{
    TOKEN_KEYWORD,
    TOKEN_IDENTIFIER,
    TOKEN_INTCONST,
    TOKEN_STRINGCONST,
    TOKEN_OPERATOR,
    TOKEN_OPENBLOCK,
    TOKEN_CLOSEBLOCK,
    TOKEN_ENDOFLINE,
    TOKEN_ERROR
} TokenType;

// Structure to represent a single token
typedef struct
{
    TokenType type;
    char value[128];
    int line;
} Token;

// Maximum number of tokens allowed in the program
#define MAX_TOKENS 1024

// Global list to store all tokens found during lexical analysis
extern Token token_list[MAX_TOKENS];

// Global counter to track the number of tokens stored
extern int token_count;

// Retrieves the next token from the input stream
Token get_next_token();

// Tokenizes the given input file and optionally writes token information to an output file
void tokenize(FILE *in, FILE *out);

#endif