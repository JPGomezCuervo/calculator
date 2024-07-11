#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "calc.h"
#include <time.h>

struct Lexer *tokens = NULL;  
struct Leaf *tree = NULL;
char *input = NULL;           
size_t input_len = 0; 

//TODO: Handle implicit multiplication eg. 2(2)
//TODO: Handle errors in continuos mode
//TODO: Add history

int main(int argsc, char **argsv)
{
        atexit(calc_cleanup);
        bool alive = true;

        /* continuous mode */
        if (argsc <= 1)
        {
                while(alive)
                {
                        printf(">> ");
                        input_len = calc_scan();
                        tokens = initialize_tokens(input_len);
                        make_tokens();
                        check_semantics();
                        tree = parse_expr(MIN_LIMIT);

                        if (tree != NULL)
                                printf("%.2f\n", eval_tree(tree));
                }
        }

        /* one expr mode */
        input_len += strlen(argsv[1]);
        /* 'input_len + 2' -> input_len + \0 + delimiter */
        input = calc_malloc(sizeof(char) * (input_len + 2));

        int input_index = 0;
        char *psrc = argsv[1];

        while (*psrc != '\0')
        {
                if (*psrc == DELIMITER)
                        dead(ERR_UNKNOWN_OPERATOR);

                if (!isspace(*psrc))
                        input[input_index++] = *psrc;
                psrc++;
        }

        input[input_index++] = DELIMITER;
        input[input_index] = '\0';
        input_len = input_index;

        input = calc_realloc(input, sizeof(char) * (input_index + 1));

        initialize_tokens(input_len);

        make_tokens();
        check_semantics();
        tree = parse_expr(MIN_LIMIT);

        if (tree != NULL)
                printf("%.2f\n", eval_tree(tree));

        return 0;
}
