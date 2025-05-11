#include <stdio.h>
#include <stdlib.h>
#include "lex.h"

int main(int argc, char** argv)
{
    if(argc < 2)
    {
        printf("Provide at least one argument\n");
        return 1;
    }

    FILE *source_file = fopen(argv[1], "r");

    if (source_file == NULL)
    {
        perror("Error opening source file");
        return 1;
    }
    fseek(source_file, 0L, SEEK_END);
    size_t file_size = ftell(source_file);
    rewind(source_file);

    char *text = malloc(file_size * sizeof(char));
    if(text == NULL)
    {
        perror("Error allocating memory");
        return 1;
    }
    size_t read_status = fread(text, sizeof(char), file_size, source_file);
    if (read_status != file_size)
    {
        perror("Error reading in source file");
        return 1;
    }
    fclose(source_file);

    token_stream* tokens = lex(text);
    if (tokens == NULL)
    {
        printf("Error lexing source file");
        return 1;
    }
    free(text);

    token_stream* ptr = tokens;
    while (ptr != NULL)
    {
        printf("Token: %s\n", ptr->token->name);
        ptr = ptr->next;
    }
    return 0;
}
