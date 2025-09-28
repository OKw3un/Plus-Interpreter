// parser.h
#ifndef PARSER_H
#define PARSER_H

// Parses the entire token stream (without parsing individual statements explicitly)
void parse();

// Parses a single statement from the token stream
void parse_statement();

#endif