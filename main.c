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
//TODO: Find a way to not creating too avoid tokens, just allocate nodes


int main(int argsc, char **argsv)
{
        int totalLength;
        int i;
        char *p_des, *p_src;
        char *str;

        atexit(calc_cleanup);

        tokens = calc_malloc(sizeof(struct Lexer));
        tokens->len = 0;
        tokens->tokens = calc_calloc(BUFF_SIZE, sizeof(struct Token));

        // input = calc_calloc(BUFF_SIZE, sizeof(char));


        if (argsc < 2)
        {
                printf("USE: calc <expr>\n");
                exit(1);
        }

        /* Joins all args into one string */
        totalLength = 0;
        for (i = 1; i < argsc; i++) 
        {
            totalLength += strlen(argsv[i]) + 1;  // +1 additional space for \0
        }

        str = malloc(totalLength);
        if (str == NULL) 
        {
            printf("malloc fail!!\n");
            return 1;
        }
        
        str[0] = '\0';
        p_des = str;
        for (i = 1; i < argsc; i++) 
        {
            p_src = argsv[i];
            while (*p_src != '\0')
            {
                if (*p_src != ' ')
                {
                    *p_des = *p_src;
                    p_des++;
                    p_src++;
                }
            }
        }

/*
        if (argsc > 1) // redundant
        {
                for (int i = 1; i < argsc; i++)
                {
                        for (size_t j = 0; j < strlen(argsv[i]); j++) // strlen will search for '\0' from the beginning of argsv[i] every loop
                        {
                                if (!isspace(argsv[i][j]))
                                        strncat(input, &argsv[i][j], 1); // strncat will search for '\0' from the beginning of input every time
                        }
                }
        } 
*/
        // input = str;
        // input_len = p_des - str;
        
        input_len = strlen(input);
        input[input_len] = (char) DELIMITER;
        input[++input_len] = '\0';

        char temp[TEMP_STR];
        temp[TEMP_STR - 1] = '\0';
        int pos = 0;
        bool was_number = false;


        for (size_t i = 0; i < input_len; i++) 
        {
                // lifetime of the variable
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
                if (isdigit(c) || c == '.')
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

        tree = parse_expr(MIN_LIMIT);

        if (tree != NULL)
                printf("%.2f\n", eval_tree(tree));

        return 0;
}
