#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"

// External variables and functions declared in lexer/parser
extern int current;
extern Token *peek();
extern Token *advance();
extern int match(TokenType type, const char *val);

#define MAX_VARS 100

// A simple structure to represent a variable in the program
typedef struct
{
    char name[64];
    long long value;
    int initialized;
} Variable;

// A table to store all declared variables
Variable var_table[MAX_VARS];
int var_count = 0;

// Declaration of the function used to interpret a single statement
extern Token token_list[MAX_TOKENS];
extern int token_count;

// Forward declaration
void interpret_statement(void);

// Function to check if a string represents an integer number
int is_integer(const char *s)
{
    if (s[0] == '-')
        s++;
    for (int i = 0; s[i]; i++)
        if (s[i] < '0' || s[i] > '9')
            return 0;
    return 1;
}

// Function to find variable index by name, -1 if not found
int find_var(const char *name)
{
    for (int i = 0; i < var_count; i++)
        if (strcmp(var_table[i].name, name) == 0)
            return i;
    return -1;
}

// Function to declare a new variable and ensure it is not already declared
void declare_var(const char *name, int line)
{
    if (find_var(name) != -1)
    {
        fprintf(stderr, "[ERROR] (line %d): Variable '%s' already declared.\n", line, name);
        exit(1);
    }
    if (var_count >= MAX_VARS)
    {
        fprintf(stderr, "[ERROR]: Too many variables.\n");
        exit(1);
    }
    strcpy(var_table[var_count].name, name);
    var_table[var_count].value = 0;
    var_table[var_count].initialized = 1;
    var_count++;
}

// Function to check that a variable exists before it is used
void check_var_exists(const char *name, int line)
{
    if (find_var(name) == -1)
    {
        fprintf(stderr, "[ERROR] (line %d): Variable '%s' is not declared.\n", line, name);
        exit(1);
    }
}

// Function to get the numeric value of a token (either constant or variable)
long long get_value(Token *t)
{
    if (t->type == TOKEN_INTCONST)
    {
        return atoll(t->value);
    }
    else if (t->type == TOKEN_IDENTIFIER)
    {
        int idx = find_var(t->value);
        if (idx == -1)
        {
            fprintf(stderr, "[ERROR] (line %d): Variable '%s' not declared.\n", t->line, t->value);
            exit(1);
        }
        return var_table[idx].value;
    }
    else
    {
        fprintf(stderr, "ERROR (line %d): Invalid value '%s'.\n", t->line, t->value);
        exit(1);
    }
}

// Function to set or update the value of a variable
void set_variable(const char *name, long long new_value)
{
    int idx = find_var(name);
    if (idx == -1)
    {
        fprintf(stderr, "[ERROR]: Variable '%s' not declared.\n", name);
        exit(1);
    }
    var_table[idx].value = new_value;
}

// Function to interpret a block of statements enclosed by { }
void interpret_block()
{
    if (!match(TOKEN_OPENBLOCK, NULL))
    {
        fprintf(stderr, "[ERROR]: Expected '{'\n");
        exit(1);
    }

    while (peek() && peek()->type != TOKEN_CLOSEBLOCK)
    {
        interpret_statement();
    }

    if (!match(TOKEN_CLOSEBLOCK, NULL))
    {
        fprintf(stderr, "[ERROR]: Expected '}' to close block.\n");
        exit(1);
    }
}

// Function to interpret a write statement: write strings, variables, constants, newline
void interpret_write()
{
    advance(); // Skip the "write" keyword

    int expect_and = 0; // Controls whether "and" keyword is expected between write elements

    while (1)
    {
        Token *t = peek(); // Look at the next token

        if (!t)
            break; // If no token, end of input

        if (expect_and)
        {
            // After a value has been printed, expect the keyword "and"
            if (!match(TOKEN_KEYWORD, "and"))
                break;      // If "and" is not found, end the write list
            expect_and = 0; // Reset flag to expect next value
        }
        else
        {
            // Handle different types of things we can write:

            // If it's a string constant, print it as-is
            if (t->type == TOKEN_STRINGCONST)
            {
                printf("%s", t->value);
                advance();
            }
            // If it's the keyword "newline", print a newline character
            else if (t->type == TOKEN_KEYWORD && strcmp(t->value, "newline") == 0)
            {
                printf("\n");
                advance();
            }
            // If it's a number constant or a variable, print its value
            else if (t->type == TOKEN_INTCONST || t->type == TOKEN_IDENTIFIER)
            {
                printf("%lld", get_value(t));
                advance();
            }
            // If none of the above, end the write statement
            else
            {
                break;
            }

            expect_and = 1; // After printing a value, expect "and" next
        }
    }

    // Every write statement must end with a semicolon
    if (!match(TOKEN_ENDOFLINE, NULL))
    {
        fprintf(stderr, "[ERROR]: Expected ';' at end of write statement.\n");
        exit(1);
    }
}

// Function to interpret a repeat loop with a count and a block or single statement
void interpret_repeat()
{
    advance(); // Skip the "repeat" keyword

    // Get the repeat count (either a number or a variable)
    Token *count_tok = advance();
    long long count = get_value(count_tok);

    // Expect the keyword "times" after the repeat count
    if (!match(TOKEN_KEYWORD, "times"))
    {
        fprintf(stderr, "[ERROR]: Expected 'times' after repeat.\n");
        exit(1);
    }

    // Decide whether to repeat a block { } or a single statement
    if (peek()->type == TOKEN_OPENBLOCK)
    {
        // Repeating a block of code
        int block_start = current; // Save position to reset for each iteration

        while (count >= 1)
        {
            current = block_start; // Reset to start of block
            interpret_block();     // Execute the block

            // If repeat count is a variable, update its value
            if (count_tok->type == TOKEN_IDENTIFIER)
            {
                set_variable(count_tok->value, count - 1);
                count = get_value(count_tok); // Get updated value
            }
            else
            {
                count--;
            }
        }

        // After loop, ensure variable (if used) is set to 0
        if (count_tok->type == TOKEN_IDENTIFIER)
        {
            set_variable(count_tok->value, 0);
        }
    }
    else
    {
        // Repeating a single statement
        int statement_start = current; // Save position to reset for each iteration

        while (count >= 1)
        {
            current = statement_start; // Reset to start of statement
            interpret_statement();     // Execute the statement

            // Update repeat count if it's a variable
            if (count_tok->type == TOKEN_IDENTIFIER)
            {
                set_variable(count_tok->value, count - 1);
                count = get_value(count_tok); // Refresh value
            }
            else
            {
                count--;
            }
        }

        // Set repeat variable to 0 at the end
        if (count_tok->type == TOKEN_IDENTIFIER)
        {
            set_variable(count_tok->value, 0);
        }
    }
}

// Function to interpret a single line of code
void interpret_statement()
{
    Token *t = peek(); // Look at the current token without consuming it
    if (!t)
        return; // No token to process, so return

    // If the token is a keyword
    if (t->type == TOKEN_KEYWORD)
    {
        // Variable declaration: number <identifier>;
        if (strcmp(t->value, "number") == 0)
        {
            advance();             // Consume "number" keyword
            Token *id = advance(); // Expect an identifier after "number"

            if (!id || id->type != TOKEN_IDENTIFIER)
            {
                // Error if identifier is missing or invalid
                fprintf(stderr, "[ERROR]: Expected identifier after 'number'\n");
                exit(1);
            }

            declare_var(id->value, id->line); // Add variable to the table

            // Expect a semicolon
            if (!match(TOKEN_ENDOFLINE, NULL))
            {
                fprintf(stderr, "[ERROR]: Expected ';' after declaration\n");
                exit(1);
            }
        }

        // Write statement: write something;
        else if (strcmp(t->value, "write") == 0)
        {
            interpret_write(); // Handle 'write' keyword and output values
        }

        // Repeat statement: repeat n times { }
        else if (strcmp(t->value, "repeat") == 0)
        {
            interpret_repeat(); // Handle "repeat" loop
        }

        // Unknown keyword
        else
        {
            fprintf(stderr, "[ERROR]: Unknown keyword '%s' at line %d\n", t->value, t->line);
            exit(1);
        }
    }

    // Assignment statement: <identifier> := <value>;
    else if (t->type == TOKEN_IDENTIFIER)
    {
        Token *id = advance();  // Get the variable being assigned
        Token *op = advance();  // Get the operator (:=, +=, -=)
        Token *rhs = advance(); // Get the right-hand side (value or identifier)

        long long value = get_value(rhs);      // Convert RHS to a numeric value
        int idx = find_var(id->value);         // Look up variable index
        check_var_exists(id->value, id->line); // Ensure the variable is declared

        // Perform assignment operation based on operator
        if (strcmp(op->value, ":=") == 0)
        {
            var_table[idx].value = value; // Direct assignment
        }
        else if (strcmp(op->value, "+=") == 0)
        {
            var_table[idx].value += value; // Increment by value
        }
        else if (strcmp(op->value, "-=") == 0)
        {
            var_table[idx].value -= value; // Decrement by value
        }
        else
        {
            // Invalid operator
            fprintf(stderr, "[ERROR]: Unknown operator '%s'\n", op->value);
            exit(1);
        }

        // Every assignment must end with a semicolon
        if (!match(TOKEN_ENDOFLINE, NULL))
        {
            fprintf(stderr, "[ERROR]: Expected ';' after assignment.\n");
            exit(1);
        }
    }

    // Block statement: { }
    else if (t->type == TOKEN_OPENBLOCK)
    {
        interpret_block(); // Recursively interpret a code block
    }

    // Invalid or unexpected token
    else
    {
        fprintf(stderr, "[ERROR]: Unexpected token '%s' on line %d\n", t->value, t->line);
        exit(1);
    }
}

// Main function to interpret all statements from the token list
void interpret()
{
    current = 0;
    while (peek())
    {
        interpret_statement();
    }
}
