#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Gets the name of the source file to be used.
void get_source_filename(int argc, char *argv[], char *filename, size_t size)
{

    // If the user provides it as a command line argument, it's used directly.
    if (argc >= 2)
    {
        snprintf(filename, size, "%s.ppp", argv[1]);
    }
    else
    {

        // Otherwise, the program asks the user to enter a name.
        char input[256];
        printf("Enter source file name (without extension): ");
        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            fprintf(stderr, "Failed to read input.\n");
            exit(1);
        }
        input[strcspn(input, "\r\n")] = 0;
        snprintf(filename, size, "%s.ppp", input); /// The ".ppp" extension is automatically added.
    }
}

// Tries to open the source file for reading.
FILE *open_source_file(const char *filename)
{
    FILE *file = fopen(filename, "r");

    // If the file cannot be opened, the program prints an error and stops.
    if (!file)
    {
        fprintf(stderr, "Could not open source file '%s'\n", filename);
        exit(1);
    }
    return file;
}
