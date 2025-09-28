#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"
#include "lexer.h"

// Forward declaration of the main parsing function for statements
void parse_statement();

// Keeps track of the current token index
int current = 0;

// Stores the last successfully consumed token and its line number
Token *last_token = NULL;
int last_token_line = -1;

// Converts token type enums strings
const char *token_type_to_string(TokenType type)
{
    switch (type)
    {
    case TOKEN_KEYWORD:
        return "keyword";
    case TOKEN_IDENTIFIER:
        return "identifier";
    case TOKEN_INTCONST:
        return "integer constant";
    case TOKEN_STRINGCONST:
        return "string constant";
    case TOKEN_OPERATOR:
        return "operator";
    case TOKEN_OPENBLOCK:
        return "open block";
    case TOKEN_CLOSEBLOCK:
        return "close block";
    case TOKEN_ENDOFLINE:
        return "semicolon";
    case TOKEN_ERROR:
        return "error token";
    default:
        return "unknown";
    }
}

// Function to peek at the current token without advancing
Token *peek()
{
    if (current < token_count)
        return &token_list[current];
    return NULL;
}

// Function to advance to the next token and returns the current one
Token *advance()
{
    if (current < token_count)
    {
        last_token = &token_list[current];
        last_token_line = last_token->line;
        return &token_list[current++];
    }
    return NULL;
}

// Function to match a token of a given type and (optional) value, then advances
int match(TokenType type, const char *val)
{
    Token *t = peek();
    if (t && t->type == type && (!val || strcmp(t->value, val) == 0))
    {
        advance();
        return 1;
    }
    return 0;
}

// Function to ensure the next token matches expected type and value, otherwise throws an error
void expect(TokenType type, const char *val)
{
    Token *t = peek();

    // If the token does not match the expected type/value, handle the error
    if (!match(type, val))
    {
        int err_line;

        if (t)
        {
            // Default error line is the current token's line
            err_line = t->line;

            // If the last token is from a previous line, the error likely belongs to that line
            if (last_token && last_token->line < t->line)
            {
                err_line = last_token->line;
            }
        }
        else
        {
            // If there's no current token fall back to the last known token's line
            err_line = last_token_line;
        }

        // Determine the string to describe what was expected
        const char *expected_str;
        if (val)
        {
            // Use the specific value, if given
            expected_str = val;
        }
        else
        {
            // Otherwise, describe based on token type
            switch (type)
            {
            case TOKEN_KEYWORD:
                expected_str = "a keyword";
                break;
            case TOKEN_IDENTIFIER:
                expected_str = "an identifier";
                break;
            case TOKEN_INTCONST:
                expected_str = "an integer constant";
                break;
            case TOKEN_STRINGCONST:
                expected_str = "a string constant";
                break;
            case TOKEN_OPERATOR:
                expected_str = "an operator";
                break;
            case TOKEN_ENDOFLINE:
                expected_str = "semicolon ';'";
                break;
            default:
                expected_str = "a token";
                break;
            }
        }

        // Print the error message with line number and found token
        fprintf(stderr, "[ERROR] (line %d): Expected token '%s' but got '%s'.\n",
                err_line,
                expected_str,
                t ? t->value : "EOF");

        // Exit the program due to syntax error
        exit(1);
    }
}

// Function to parse: number <identifier>;
void parse_declaration()
{
    expect(TOKEN_KEYWORD, "number");
    expect(TOKEN_IDENTIFIER, NULL);
    expect(TOKEN_ENDOFLINE, NULL);
}

// Function to parse assignment statements: <identifier> := <value>;
void parse_assignment()
{
    // Expect an identifier as the target of the assignment
    expect(TOKEN_IDENTIFIER, NULL);

    // Expect the assignment operator :=
    expect(TOKEN_OPERATOR, ":=");

    // Peek at the next token to see if it's a valid value (int or identifier)
    Token *t = peek();
    if (t && (t->type == TOKEN_INTCONST || t->type == TOKEN_IDENTIFIER))
    {
        advance(); // Consume the value token
    }
    else
    {
        // Determine the appropriate line for the error message
        int err_line = t ? t->line : last_token_line;
        if (last_token && t && last_token->line < t->line)
        {
            err_line = last_token->line;
        }

        // Report a syntax error if value is missing or invalid
        fprintf(stderr, "[ERROR] (line %d): Expected int or identifier in assignment.\n", err_line);
        exit(1);
    }

    // Expect semicolon at the end of the statement
    expect(TOKEN_ENDOFLINE, NULL);
}

// Function to parse increment statements: <identifier> += <value>;
void parse_increment()
{
    // Expect an identifier before the += operator
    expect(TOKEN_IDENTIFIER, NULL);

    // Expect the increment operator +=
    expect(TOKEN_OPERATOR, "+=");

    // Peek and validate the value (either int or identifier)
    Token *t = peek();
    if (t && (t->type == TOKEN_INTCONST || t->type == TOKEN_IDENTIFIER))
    {
        advance(); // Consume the value
    }
    else
    {
        // Determine the line for reporting the error
        int err_line = t ? t->line : last_token_line;
        if (last_token && t && last_token->line < t->line)
        {
            err_line = last_token->line;
        }

        // Print syntax error message
        fprintf(stderr, "[ERROR] (line %d): Expected int or identifier in increment.\n", err_line);
        exit(1);
    }

    // Expect a semicolon to terminate the statement
    expect(TOKEN_ENDOFLINE, NULL);
}

// Function to parse decrement statements: <identifier> -= <value>;
void parse_decrement()
{
    // Expect an identifier before the -= operator
    expect(TOKEN_IDENTIFIER, NULL);

    // Expect the decrement operator -=
    expect(TOKEN_OPERATOR, "-=");

    // Validate the value after -= (should be int or identifier)
    Token *t = peek();
    if (t && (t->type == TOKEN_INTCONST || t->type == TOKEN_IDENTIFIER))
    {
        advance(); // Consume the value
    }
    else
    {
        // Select appropriate line for error reporting
        int err_line = t ? t->line : last_token_line;
        if (last_token && t && last_token->line < t->line)
        {
            err_line = last_token->line;
        }

        // Report a syntax error for invalid decrement value
        fprintf(stderr, "[ERROR]: (line %d): Expected int or identifier in decrement.\n", err_line);
        exit(1);
    }

    // Ensure statement ends with a semicolon
    expect(TOKEN_ENDOFLINE, NULL);
}

// Function to parse: write <value> [and <value>]*;
void parse_write()
{
    // Expect the 'write' keyword at the beginning
    expect(TOKEN_KEYWORD, "write");

    int expect_and = 0; // Flag to track whether 'and' is expected between values

    while (1)
    {
        Token *t = peek();

        // If there are no more tokens, it's an unexpected EOF
        if (!t)
        {
            int err_line = last_token_line;
            fprintf(stderr, "[ERROR] (line %d): Unexpected end of input in write statement.\n", err_line);
            exit(1);
        }

        if (expect_and)
        {
            // If "and" is expected but not found, it means the write list has ended
            if (!match(TOKEN_KEYWORD, "and"))
            {
                break; // Exit loop if there's no 'and' keyword
            }
            expect_and = 0; // After "and", expect another value
        }
        else
        {
            // Check if the next token is a valid printable value
            if (t->type == TOKEN_STRINGCONST ||
                t->type == TOKEN_INTCONST ||
                t->type == TOKEN_IDENTIFIER ||
                (t->type == TOKEN_KEYWORD && strcmp(t->value, "newline") == 0))
            {
                advance();      // Consume the value
                expect_and = 1; // After a value, "and" may follow
            }
            else
            {
                // If an invalid token is encountered, report an error
                fprintf(stderr, "[ERROR] (line %d): Unexpected token '%s' in write statement. Expected string, identifier, or newline.\n",
                        t->line, t->value);
                exit(1);
            }
        }
    }

    // Ensure the statement ends properly with a semicolon
    expect(TOKEN_ENDOFLINE, NULL);
}

// Function to parse block of statements between { and }
void parse_block()
{
    // Expect the opening block symbol '{'
    expect(TOKEN_OPENBLOCK, NULL);

    while (1)
    {
        Token *t = peek();

        // If there are no more tokens, it's an unexpected end of input (missing '}')
        if (!t)
        {
            fprintf(stderr, "[ERROR]: Unexpected end of input in block.\n");
            exit(1);
        }

        // If the closing block symbol '}' is found, consume it and exit the loop
        if (t->type == TOKEN_CLOSEBLOCK)
        {
            expect(TOKEN_CLOSEBLOCK, NULL);
            break;
        }

        // Otherwise, parse the next statement inside the block
        parse_statement();
    }
}

// Function to parse: repeat <value> times { ... } OR single statement
void parse_repeat()
{
    // Expect "repeat" keyword
    expect(TOKEN_KEYWORD, "repeat");

    // Expect an integer constant or identifier as the repeat count
    Token *t = peek();
    if (t && (t->type == TOKEN_INTCONST || t->type == TOKEN_IDENTIFIER))
    {
        advance(); // Consume the count token
    }
    else
    {
        fprintf(stderr, "[ERROR] (line %d): Expected int or identifier after 'repeat'.\n", t ? t->line : -1);
        exit(1);
    }

    // Expect the "times" keyword following the count
    expect(TOKEN_KEYWORD, "times");

    // Check what's coming next: either a block or a single statement
    t = peek();
    if (t && t->type == TOKEN_OPENBLOCK)
    {
        // If it's a block, parse the entire block
        parse_block();
    }
    else
    {
        // Handle case where repeat is followed by a single statement
        if (!t)
        {
            fprintf(stderr, "[ERROR]: Unexpected end of input after 'repeat times'.\n");
            exit(1);
        }

        // Handle inline "write" statement
        if (t->type == TOKEN_KEYWORD && strcmp(t->value, "write") == 0)
        {
            parse_write();
        }
        // Handle assignment, increment, or decrement statements
        else if (t->type == TOKEN_IDENTIFIER)
        {
            // Look ahead to determine what kind of statement this is
            Token *lookahead = &token_list[current + 1];
            if (lookahead->type == TOKEN_OPERATOR && strcmp(lookahead->value, ":=") == 0)
            {
                parse_assignment();
                expect(TOKEN_ENDOFLINE, NULL); // Ensure line ends properly
            }
            else if (lookahead->type == TOKEN_OPERATOR && strcmp(lookahead->value, "+=") == 0)
            {
                parse_increment();
                expect(TOKEN_ENDOFLINE, NULL);
            }
            else if (lookahead->type == TOKEN_OPERATOR && strcmp(lookahead->value, "-=") == 0)
            {
                parse_decrement();
                expect(TOKEN_ENDOFLINE, NULL);
            }
            else
            {
                fprintf(stderr, "[ERROR] (line %d): Unexpected token after 'repeat times'.\n", t->line);
                exit(1);
            }
        }
        else
        {
            // If token is not one of the expected types
            fprintf(stderr, "[ERROR] (line %d): Unexpected token after 'repeat times'.\n", t->line);
            exit(1);
        }
    }
}

// Function of debug utility to print all tokens
void debug_tokens()
{
    printf("\n--- Token List ---\n");
    for (int i = 0; i < token_count; i++)
    {
        printf("Line %d: %-15s %s\n", token_list[i].line, token_type_to_string(token_list[i].type), token_list[i].value);
    }
    printf("------------------\n");
}

// Function to parse a single statement
void parse_statement()
{
    Token *t = peek();
    if (!t)
        return; // No more tokens to parse

    // Handle keyword-based statements
    if (t->type == TOKEN_KEYWORD)
    {
        if (strcmp(t->value, "number") == 0)
        {
            // Variable declaration
            parse_declaration();
        }
        else if (strcmp(t->value, "write") == 0)
        {
            // Output statement
            parse_write();
        }
        else if (strcmp(t->value, "repeat") == 0)
        {
            // Looping statement
            parse_repeat();
        }
        else
        {
            // Unknown keyword
            fprintf(stderr, "[ERROR] (line %d): Unexpected keyword '%s'\n", t->line, t->value);
            exit(1);
        }
    }

    // Handle identifier-based statements (e.g., assignments)
    else if (t->type == TOKEN_IDENTIFIER)
    {
        // Look ahead to determine which type of operation this is
        Token *lookahead = &token_list[current + 1];
        if (lookahead->type == TOKEN_OPERATOR)
        {
            if (strcmp(lookahead->value, ":=") == 0)
            {
                // Assignment
                parse_assignment();
            }
            else if (strcmp(lookahead->value, "+=") == 0)
            {
                // Increment
                parse_increment();
            }
            else if (strcmp(lookahead->value, "-=") == 0)
            {
                // Decrement
                parse_decrement();
            }
            else
            {
                // Unknown operator after identifier
                fprintf(stderr, "[ERROR] (line %d): Unexpected operator '%s'\n", lookahead->line, lookahead->value);
                exit(1);
            }
        }
        else
        {
            // Identifier not followed by valid operator
            fprintf(stderr, "[ERROR] (line %d): Unexpected token '%s'\n", t->line, t->value);
            exit(1);
        }
    }

    // Handle opening block
    else if (t->type == TOKEN_OPENBLOCK)
    {
        // Nested block statement
        parse_block();
    }

    // Unmatched closing block
    else if (t->type == TOKEN_CLOSEBLOCK)
    {
        fprintf(stderr, "[ERROR] (line %d): Unexpected '}'\n", t->line);
        exit(1);
    }

    // Any other unexpected token
    else
    {
        fprintf(stderr, "[ERROR] (line %d): Unexpected token '%s'\n", t->line, t->value);
        exit(1);
    }
}

// Function to parse entire token stream
void parse()
{
    while (current < token_count)
    {
        // Parse a single statement at the current token position
        parse_statement();
    }
    // If all tokens have been parsed without errors, print success message
    printf("Syntax analysis completed successfully.\n");
}
