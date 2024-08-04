#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "calc.h"

/* THIS IS AN EXAMPLE ON HOW TO USE THE LIBRARY */
int main(int argsc, char **argsv)
{
        /* number of history elements */
        Calculator *calculator = init_calculator(5);

        if (argsc > 1)
        {
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
                                                history[i]->result);
                        }
                }
        }
        else
        {
                char *line;
                        printf("Entering interactive mode...\n");
                while ((line = readline(">> ")) != NULL)
                {
                        if (*line)
                        {
                                add_history(line);
                                double result = calculate_expr(calculator, line);

                                if (!get_error_code(calculator))
                                        printf("%.2f\n", result);
                        }
                        free(line);
                }
                if (line == NULL)
                {
                        free(line);
                        printf("\nExiting interactive mode...\n");
                }
        }

        destroy_calculator(calculator);
        return 0;
}
