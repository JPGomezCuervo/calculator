#include <stdio.h>
#include "calc.h"

//TODO: Handle implicit multiplication eg. 2(2)
int main(int argsc, char **argsv)
{
        (void) argsc;
        /* number of history elements */
        Calculator *calculator = init_calculator(5);

        double result = calculate_expr(calculator, argsv[1]);
        Expression **history = get_history(calculator);

        if (!error_code(calculator))
        {
                printf("%.2f\n", result);
                printf("HISTORY\n");
                for (size_t i = 0; i < get_history_len(calculator); i++)
                {
                        printf("id: %d, expr: %s, res: %.2f\n",
                                        history[i]->id,
                                        history[i]->expr,
                                        history[i]->result
                                        );
                }
        }

        destroy_calculator(calculator);
}
