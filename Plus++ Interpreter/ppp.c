#include <stdio.h>
#include "file_utils.h"
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

void debug_tokens();

int main(int argc, char *argv[])
{
    char source_file[512];

    // Get the source filename from command line arguments or prompt the user
    get_source_filename(argc, argv, source_file, sizeof(source_file));

    // Open the source file for reading
    FILE *infile = open_source_file(source_file);

    // Tokenize the input source file and output tokens to stdout
    tokenize(infile, stdout);
    fclose(infile);

    // Optional: print or inspect tokens for debugging
    debug_tokens();

    // Parse the token stream into syntax structures
    parse();

    // Interpret the parsed code
    interpret();

    return 0;
}