#include <stdio.h>
#include "calc.h"

/* THIS IS AN EXAMPLE ON HOW TO USE THE LIBRARY */
int main(int argsc, char **argsv)
{
        (void) argsc;
        /* number of history elements */
        Calculator *calculator = init_calculator(5);

        if (!calculator)
        {
                printf("Failed to initialize calculator\n");
                return 1;
        }

        double result = calculate_expr(calculator, argsv[1]);
        Expression **history = get_history(calculator);

        if (!get_error_code(calculator))
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
