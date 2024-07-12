#include <stdio.h>
#include <stdbool.h>
#include "calc.h"

//TODO: Handle implicit multiplication eg. 2(2)
int main(int argsc, char **argsv)
{
        if (argsc != 2)
        {
                printf("USE: calc <expr>\n");
                dead(ERR_NO_INPUT);
        }

        Calculator *calculator = init_calculator();
        double result = calculate_expr(calculator, argsv[1]);

        printf("%.2f\n", result);
}
