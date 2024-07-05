#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "calc.h"
#include <time.h>

#define TEMP_STR 51

struct Lexer *tokens = NULL;  
struct Leaf *tree = NULL;
char *input = NULL;           
size_t input_len = 0; 

//TODO: Define error codes
//TODO: Handle errors when input empty
//TODO: Check errors before entry to the functions
//TODO: Add continuous mode when no args are passed, big while

int main(int argsc, char **argsv)
{
        atexit(calc_cleanup);

        clock_t start, end;
        double cpu_time_used;

        start = clock();

        if (argsc < 2)
        {
                printf("USE: calc <expr>\n");
                exit(1);
        }


        /* Joins all args into one string */

        for (int i = 1; i < argsc; i++)
                input_len += strlen(argsv[i]);


        input = calc_malloc(sizeof(char) * (input_len + 1));

        {
                int input_index = 0;

                for (int i = 1; i < argsc; i++)
                {
                        char *psrc = argsv[i];

                        while (*psrc != '\0')
                        {
                                if (!isspace(*psrc))
                                        input[input_index++] = *psrc;
                                psrc++;
                        }
                }

                input[input_index++] = DELIMITER;
                input[input_index] = '\0';
                input_len = input_index;

                input = calc_realloc(input, sizeof(char) * (input_index + 1));
        }

        tokens = calc_malloc(sizeof(struct Lexer));
        tokens->len = input_len;
        tokens->chars = calc_malloc(sizeof(char*) * input_len);


        size_t char_pos = 0;
        for (size_t i = 0; i < input_len; i++)
        {
                enum Type t = get_type(input[i]);
                add_token(tokens, input, &i, t, input_len, char_pos);
                char_pos++;
        }
        tokens->chars = calc_realloc(tokens->chars, sizeof(char*) * char_pos);
        tokens->len = char_pos;
        tokens->curr = 0;


        tree = parse_expr(MIN_LIMIT);

        if (tree != NULL)
                printf("%.2f\n", eval_tree(tree));


        end = clock();
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        printf("Tiempo de CPU usado: %f segundos\n", cpu_time_used);

        return 0;
}
