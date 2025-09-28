#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

Token token_list[MAX_TOKENS];
int token_count = 0;
int peekc;

// List of keywords used in the language.
const char *keywords[] = {
    "number", "repeat", "times", "write", "newline", "and", NULL};

// Function to check if an IntConstant is longer than 100 digits
void check_intconstant_length(const char *num, int line)
{
    // Do not count if it has a sign
    int len = strlen(num);
    if (num[0] == '-' || num[0] == '+')
    {
        len--;
    }
    if (len > 100)
    {
        fprintf(stderr, "[ERROR]: (Line %d): IntConstant exceeds 100 digits.\n", line);
        exit(1);
    }
}

// Function to check if an Identifier is longer than 20 characters
void check_identifier_length(const char *name, int line)
{
    if (strlen(name) > 20)
    {
        fprintf(stderr, "[ERROR]: (Line %d): Identifier exceeds 20 characters.\n", line);
        exit(1);
    }
}

// Function to check if the given word is a keyword. Returns 1 if it is a keyword, 0 otherwise.
int is_keyword(const char *word)
{
    for (int i = 0; keywords[i]; i++)
    {
        if (strcmp(word, keywords[i]) == 0)
            return 1;
    }
    return 0;
}

// Function to check if a character is valid as the first character of an identifier. Identifiers can start with a letter or underscore (_).
int is_identifier_start(char c)
{
    return isalpha(c) || c == '_';
}

// Function to check if a character is valid inside an identifier. Identifiers can contain letters, digits, or underscore.
int is_identifier_char(char c)
{
    return isalnum(c) || c == '_';
}

// Maximum number of identifiers that can be stored.
#define MAX_IDENTIFIERS 256
// Array to store the names of declared identifiers (for example variable names).
char *declared_identifiers[MAX_IDENTIFIERS];
// Keeps track of how many identifiers have been declared so far.
int declared_count = 0;

// Function to check if an identifier with the given name has already been declared. Returns 1 if it is found, 0 otherwise.
int is_declared(const char *name)
{
    for (int i = 0; i < declared_count; i++)
    {
        if (strcmp(declared_identifiers[i], name) == 0)
            return 1;
    }
    return 0;
}

// Function to add a new identifier to the declared list. Only adds if there's space left in the array.
void declare_identifier(const char *name)
{
    if (declared_count < MAX_IDENTIFIERS)
    {
        declared_identifiers[declared_count++] = strdup(name); // Store a copy of the name
    }
}

// Function to write a token to the output file in the format: TYPE(VALUE) If value is NULL, writes only the type.
void write_token(FILE *out, const char *type_str, const char *value_str, int line)
{

    if (token_count >= MAX_TOKENS)
    {
        fprintf(stderr, "[ERROR]: Too many tokens.\n");
        exit(1);
    }

    Token *t = &token_list[token_count++];

    // Set token type
    if (strcmp(type_str, "Keyword") == 0)
        t->type = TOKEN_KEYWORD;
    else if (strcmp(type_str, "Identifier") == 0)
        t->type = TOKEN_IDENTIFIER;
    else if (strcmp(type_str, "IntConstant") == 0)
        t->type = TOKEN_INTCONST;
    else if (strcmp(type_str, "StringConstant") == 0)
        t->type = TOKEN_STRINGCONST;
    else if (strcmp(type_str, "Operator") == 0)
        t->type = TOKEN_OPERATOR;
    else if (strcmp(type_str, "OpenBlock") == 0)
        t->type = TOKEN_OPENBLOCK;
    else if (strcmp(type_str, "CloseBlock") == 0)
        t->type = TOKEN_CLOSEBLOCK;
    else if (strcmp(type_str, "EndOfLine") == 0)
        t->type = TOKEN_ENDOFLINE;
    else
        t->type = TOKEN_ERROR;

    // Set token value (if any)
    if (value_str)
        strncpy(t->value, value_str, sizeof(t->value) - 1);
    else
        t->value[0] = '\0';

    t->line = line;

    // Write token to file
    if (value_str)
        fprintf(out, "%s(%s)\n", type_str, value_str);
    else
        fprintf(out, "%s\n", type_str);
}

// Main tokenizer function. Reads characters from the input file and writes tokens to the output file.
void tokenize(FILE *in, FILE *out)
{
    int c;
    int line = 1;                          // Track current line number for error reporting.
    int expect_identifier_declaration = 0; // After "number", expect an identifier.

    // Main loop: read the input file one character at a time.
    while ((c = fgetc(in)) != EOF)
    {

        // Handle newlines to track line numbers.
        if (c == '\n')
        {
            line++;
            fprintf(stderr, "Debug: Line %d\n", line); // Optional: print current line number
        }

        // If block to search and write End Of Line token
        if (c == ';')
        {
            write_token(out, "EndOfLine", NULL, line);
            continue;
        }

        // If block to search and write Comment token
        if (c == '*')
        {
            int comment_closed = 0;
            int comment_start_line = line;
            while ((c = fgetc(in)) != EOF)
            {
                if (c == '\n')
                    line++; // Allow multiline comments
                if (c == '*')
                {
                    comment_closed = 1;
                    break; // End of comment
                }
            }
            if (!comment_closed)
            {
                // Report error if comment is not closed
                fprintf(stderr, "[ERROR]: Unterminated comment detected at line %d\n", comment_start_line);
                write_token(out, "Error", "Unterminated comment detected.", line);
                exit(1);
            }
            continue;
        }

        // If block to search and write Open Bracket token
        if (c == '{')
        {
            write_token(out, "OpenBlock", NULL, line);
            continue;
        }

        // If block to search and write Close Bracket token
        if (c == '}')
        {
            write_token(out, "CloseBlock", NULL, line);
            continue;
        }

        // String constants enclosed in double quotes: "..."
        if (c == '\"')
        {
            char str[256] = {0};
            int i = 0;
            int str_closed = 0;
            int str_line = line;

            while ((c = fgetc(in)) != EOF && i < 255)
            {
                if (c == '\"')
                {
                    str_closed = 1;
                    break; // Closing quote found
                }
                if (c == '\n')
                {
                    line++; // Strings cannot span lines
                }
                str[i++] = c;
            }
            str[i] = '\0';

            if (str_closed)
            {
                write_token(out, "StringConstant", str, line);
            }
            else
            {
                // Unterminated string literal
                fprintf(stderr, "[ERROR] (Line %d): Unterminated string constant.\n", str_line);
                write_token(out, "Error", "Unterminated string constant.", line);
                exit(1);
            }
            continue;
        }

        // Operator or negative int constant
        if (c == ':' || c == '+' || c == '-')
        {
            int next = fgetc(in);

            // Dual operator control
            if (c == ':' && next == '=')
            {
                write_token(out, "Operator", ":=", line);
            }
            else if (c == '+' && next == '=')
            {
                write_token(out, "Operator", "+=", line);
            }
            else if (c == '-' && next == '=')
            {
                write_token(out, "Operator", "-=", line);
            }
            // Signed number control
            else if ((c == '-' || c == '+') && isdigit(next))
            {
                // Signed number starts
                char num[128];
                int i = 0;
                num[i++] = c;
                num[i++] = next;

                while ((next = fgetc(in)) != EOF && isdigit(next))
                {
                    num[i++] = next;
                }

                num[i] = '\0';
                ungetc(next, in);

                check_intconstant_length(num, line);
                write_token(out, "IntConstant", num, line);
            }
            else
            {
                // Single operator
                ungetc(next, in);
                char op[2] = {c, '\0'};
                write_token(out, "Operator", op, line);
            }
            continue;
        }

        // IntConstant that starts only with integer
        if (isdigit(c))
        {
            char num[128];
            int i = 0;

            num[i++] = c;
            while ((c = fgetc(in)) != EOF && isdigit(c))
            {
                num[i++] = c;
            }

            num[i] = '\0';
            ungetc(c, in);

            check_intconstant_length(num, line);
            write_token(out, "IntConstant", num, line);
            continue;
        }

        // If block to search and write Identifiers (variable names) or keywords
        if (is_identifier_start(c))
        {
            char word[64];
            int i = 0;
            word[i++] = c;
            while ((c = fgetc(in)) != EOF && is_identifier_char(c))
            {
                word[i++] = c;
            }
            word[i] = '\0';
            ungetc(c, in); // Return the non-identifier character

            check_identifier_length(word, line);

            // Check if the word is a keyword
            if (is_keyword(word))
            {
                write_token(out, "Keyword", word, line);
                // If it's the "number" keyword, expect an identifier next
                if (strcmp(word, "number") == 0)
                {
                    expect_identifier_declaration = 1;
                }
            }
            else
            {
                // Handle identifiers
                if (expect_identifier_declaration)
                {
                    declare_identifier(word); // Add to declared list
                    write_token(out, "Identifier", word, line);
                    expect_identifier_declaration = 0;
                }
                else if (is_declared(word))
                {
                    write_token(out, "Identifier", word, line);
                }
                else
                {
                    // Undeclared identifier used — this is an error
                    char err_msg[128];
                    sprintf(err_msg, "'%s' is not defined", word);
                    fprintf(stderr, "[ERROR] (Line %d): %s\n", line, err_msg);
                    write_token(out, "Error", err_msg, line);
                    exit(1);
                }
            }
            continue;
        }

        // If block to search and write any other character that is not whitespace is considered an error
        if (!isspace(c))
        {
            char err_msg[64];
            sprintf(err_msg, "[ERROR]: Unrecognized character '%c'", c);
            fprintf(stderr, "[ERROR] (Line %d): %s\n", line, err_msg);
            write_token(out, "Error", err_msg, line);
            exit(1);
        }
    }
}