#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <stdio.h>

// Function to get the name of the source file to be used in the program.
void get_source_filename(int argc, char *argv[], char *filename, size_t size);

// Function to open the given source file for reading.
FILE *open_source_file(const char *filename);

#endif