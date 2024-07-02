#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "calc.h"

#define BUFF_SIZE 101
#define DELIMITER '?'
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

        tokens = calc_malloc(sizeof(struct Lexer));
        tokens->len = 0;
        tokens->tokens = calc_calloc(BUFF_SIZE, sizeof(struct Token));

        input = calc_calloc(BUFF_SIZE, sizeof(char));


        if (argsc < 2)
        {
                printf("USE: calc <expr>\n");
                exit(1);
        }


        /* Joins all args into one string */
        if (argsc > 1)
        {
                for (int i = 1; i < argsc; i++)
                {
                        for (size_t j = 0; j < strlen(argsv[i]); j++) 
                        {
                                if (!isspace(argsv[i][j]))
                                        strncat(input, &argsv[i][j], 1);
                        }
                }
        } 

        input_len = strlen(input);
        input[input_len] = (char) DELIMITER;
        input[++input_len] = '\0';

        char temp[TEMP_STR];
        temp[TEMP_STR - 1] = '\0';
        int pos = 0;
        bool was_number = false;


        for (size_t i = 0; i < input_len; i++) 
        {
                char c = input[i];
                enum Type t = get_type(c);
                enum Bp bp = get_bp(c);

                /* Handle prefix */
                if (i == 0 && (is_operator(t) || is_parenthesis(t))) 
                {
                        if (c == '-')
                                t = UNARY_NEG;

                        if (c == '+')
                                t = UNARY_POS;

                        if (c == '(')
                                t = OPEN_PARENT;

                        if (c == ')')
                                t = CLOSE_PARENT;

                        temp[0] = c;
                        add_token(temp, t, bp);
                        continue;
                }

                /* Store char into a buffer until another operator is found */
                if (isdigit(c))
                        handle_number(&was_number, temp, &pos, c);

                if (is_operator(t) || is_parenthesis(t))
                {
                        if (was_number)
                                handle_number_end(&was_number, temp, &pos);

                        temp[0] = c;
                        add_token(temp, t, bp);
                }
                if (t == LIMIT)
                {
                        if (was_number)
                        {
                                temp[pos] = '\0';
                                handle_number_end(&was_number, temp, &pos);
                        }
                }
        }


        // debug_tokens(tokens);
        tree = parse_expr(MIN_LIMIT);
        // debug_tree(tree, "");

        if (tree != NULL)
                printf("result: %.2f\n", eval_tree(tree));

        return 0;
}
