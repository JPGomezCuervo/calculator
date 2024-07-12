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

int main(int argsc, char **argsv)
{
        if (argsc < 2)
        {
                printf("USE: calc <expr>\n");
                dead(ERR_NO_INPUT);
        }

        Calculator *calculator = init_calculator();
        double result = calculate_expr(calculator, argsv[1]);

        printf("%.2f\n", result);
}
