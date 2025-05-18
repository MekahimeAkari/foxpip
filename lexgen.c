#include <stdio.h>

#define BUF_SIZE 1024

typedef struct _token_def {
    char* token_name;
    char* token_expr;
} token_def;


void usage(const char *prog_name)
{
    printf("%s: Lexer generator\n\n"
           "Usage: %s LEXER_FILE\n"
           "LEXER_FILE: File that contains lexer generation expressions (required)\n"
           , prog_name, prog_name);
}

size_t lexstrnlen(const char* str, size_t max_len)
{
    size_t i = 0;
    if (str == NULL)
        return 0;
    for (i = 0; i < max_len; i++)
    {
        if (str[i] == '\0')
            return i;
    }
    return 0;
}

int create_matcher(char* line, size_t name_start, size_t name_end, size_t expr_start, size_t expr_end)
{
    size_t i = 0;
    size_t match_group = 0;
    int in_pattern = 0;
    int end_pattern = 0;
    size_t pattern_start = 0;
    size_t pattern_end = 0;
    int in_range = 0;
    char start_range = '\0';

    char pattern_buf[BUF_SIZE];
    size_t pattern_buf_pos = 0;

    printf("if (*state == START)\n");
    printf("\t*state = %s_STATE_%lu;\n", line+name_start, 0UL);

    for (i = expr_start; i < expr_end; i++)
    {
        char c = line[i];
        if (c == '[' && i+1 < expr_end)
        {
            in_pattern = 1;
            pattern_start = i+1;
            pattern_end = pattern_start;
            pattern_buf_pos = 0;
        }
        else if (in_pattern == 1)
        {
            if (c == ']')
            {
                in_pattern = 0;
                end_pattern = 1;
            }
            else if (c == '-')
            {
                start_range = line[i - 1];
                in_range = 1;
            }
            else if (in_range == 1)
            {
                in_range = 0;
                char range_c = '\0';

                for (range_c = start_range; range_c <= line[i]; range_c++)
                {
                    pattern_buf[pattern_buf_pos] = range_c;
                    pattern_buf_pos++;
                }
                pattern_buf[pattern_buf_pos] = '\0';
            }
            else
            {
                pattern_end++;
            }
        }
        else
        {
            pattern_buf[0] = c;
            pattern_buf[1] = '\0';
            end_pattern = 1;
        }
        if (end_pattern == 1)
        {
            size_t pattern_i = 0;
            printf("switch (c)\n{\n");
            for (pattern_i = 0; pattern_i < BUF_SIZE; pattern_i++)
            {
                if (pattern_buf[pattern_i] == '\0')
                    break;
                printf("'%c':\n", pattern_buf[pattern_i]);
            }
            if (line[i+1] == '+')
            {
                printf("1 or more\n");
                printf("\t*state = %s_STATE_%lu_PLUS;\n\tbreak;\n", line+name_start, match_group);
                printf("default:\n");
                printf("\t{");
                printf("\tif (*state == %s_STATE_%lu)\n", line+name_start, match_group);
                printf("\t\t*state = NO_MATCH;\n\t\tbreak;\n");
                printf("\telse\n");
                printf("\t\t*state = %s_STATE_%lu;\n\t\tbreak;\n", line+name_start, match_group+1);
                printf("\t}\n");
            }
            else if (line[i+1] == '*')
            {
                printf("\t*state = %s_STATE_%lu;\n\tbreak;\n", line+name_start, match_group);
                printf("default:\n");
                printf("\t*state = %s_STATE_%lu;\n\tbreak;\n", line+name_start, match_group+1);
            }
            else
            {
                printf("\t*state = %s_STATE_%lu;\n\tbreak;\n", line+name_start, match_group+1);
                printf("default:\n");
                printf("\t*state = NO_MATCH;\n\tbreak;\n");
            }
            printf("}\n");
            end_pattern = 0;
            match_group++;
        }

    }
    return 0;
}

int extract_element(char* line)
{
    typedef enum _ele_pos {
        ELE_POS_NAME,
        ELE_POS_EXPR
    } ele_pos;

    size_t name_start = 0;
    size_t name_end = 0;
    size_t expr_start = 0;
    size_t expr_end = 0;
    size_t line_pos = 0;
    size_t line_len = lexstrnlen(line, BUF_SIZE);
    ele_pos pos = ELE_POS_NAME;
    int found_open_quote = 0;
    int found_name = 0;
    int found_expr = 0;
    int trim_leading = 1;

    if (line == NULL)
    {
        printf("Null line pointer\n");
        return 1;
    }

    for (line_pos = 0; line_pos < line_len; line_pos++)
    {
        char c = line[line_pos];
        if (found_name == 1 && found_expr == 1)
        {
            printf("Error: Extra characters on the line after closing quote\n");
            return 1;
        }
        if (pos == ELE_POS_NAME)
        {
            if (trim_leading == 1)
            {
                if (c != ' ' && c != '\t')
                {
                    trim_leading = 0;
                    name_end = name_start;
                }
                else
                    name_start++;
            }
            if (trim_leading == 0)
            {
                if (c == ':')
                {
                    line[name_end] = '\0';
                    printf("Name: %s\n", line+name_start);
                    pos = ELE_POS_EXPR;
                    found_name = 1;
                    trim_leading = 1;
                    expr_start = name_end+1;
                }
                else
                {
                    name_end++;
                }
            }
        }
        else if (pos == ELE_POS_EXPR)
        {
            if (trim_leading == 1)
            {
                if (c != ' ' && c != '\t')
                {
                    trim_leading = 0;
                    expr_end = expr_start;
                }
                else
                    expr_start++;
            }
            if (trim_leading == 0)
            {
                if (found_open_quote == 0)
                {
                    if (c == '"')
                    {
                        found_open_quote = 1;
                        expr_start++;
                    }
                    else
                    {
                        printf("Expected opening quote, not '%c'\n", c);
                        return 1;
                    }
                }
                else
                {
                    if (c == '"')
                    {
                        expr_end++;
                        line[expr_end] = '\0';
                        printf("Expr: %s\n", line+expr_start);
                        found_expr = 1;
                    }
                    else
                    {
                        expr_end++;
                    }
                }
            }
        }
        else
        {
            printf("Unknown element position\n");
            return 1;
        }
    }
    if (found_name != 1 || found_expr != 1)
    {
        printf("Error parsing line: ");
        if (found_name != 1)
            printf("missing ':'\n");
        else if (found_expr != 1)
            printf("missing '\"'\n");
        else
            printf("unknown error\n");
        return 1;
    }
    return create_matcher(line, name_start, name_end, expr_start, expr_end);
}

int extract_lines(FILE* file)
{
    char line_buf[BUF_SIZE];
    int buf_pos = 0;
    int res = 0;
    int empty_line = 1;

    if (file == NULL)
    {
        printf("Null file pointer\n");
        return 1;
    }

    do
    {
        size_t read_chars = fread(line_buf+buf_pos, 1, 1, file);
        char c = line_buf[buf_pos];

        if (feof(file))
        {
            break;
        }
        else if (read_chars != 1 || ferror(file))
        {
            perror("Error reading file");
            return 1;
        }
        if (c == '\n' || c == '\r')
        {
            line_buf[buf_pos] = '\0';
            if (empty_line != 1)
            {
                res = extract_element(line_buf);
                if (res != 0)
                    break;
            }
            empty_line = 1;
            buf_pos = 0;
        }
        else
        {
            if (empty_line == 1 && (c != ' ' && c != '\t'))
                empty_line = 0;
            if (empty_line == 0)
                buf_pos++;
        }
        if (buf_pos >= BUF_SIZE - 2)
        {
            printf("Buffer overrun\n");
            return 1;
        }
    }
    while (!feof(file));

    return res;
}

int main(int argc, char **argv)
{
    char* file_path = NULL;
    FILE* file = NULL;

    if (argc != 2)
    {
        usage(argv[0]);
        return 1;
    }

    file_path = argv[1];
    file = fopen(file_path, "r");
    if (file == NULL)
    {
        perror("Error opening file");
        return 1;
    }

    return extract_lines(file);
}
