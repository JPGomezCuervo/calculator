#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <string.h>
#include "calc.h"

#define HISTORY_SIZE 100

char *replace_id_with_value(struct Calculator *handler, char *line);

/* THIS IS AN EXAMPLE ON HOW TO USE THE LIBRARY
 * nontheless it can be used as a command line calculator */
int main(int argsc, char **argsv)
{
        Calculator *calculator = init_calculator(HISTORY_SIZE);

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
                                printf("id: %zu, expr: %s, res: %.2f\n",
                                                history[i]->id,
                                                history[i]->expr,
                                                history[i]->result);
                        }
                }
        }
        else
        {
                char *line;
                size_t history_len;
                Expression **history = get_history(calculator);

                while ((line = readline(">> ")) != NULL)
                {
                        if (*line)
                        {
                                add_history(line);
                                char *new_line = replace_id_with_value(calculator, line);
                                double result = calculate_expr(calculator, new_line);
                                history_len = get_history_len(calculator);

                                if (!get_error_code(calculator))
                                        printf("$%zu = %.2f\n",history[history_len - 1]->id, result);

                                if (new_line != line)
                                        free(new_line);
                        }
                        free(line);
                }
                if (line == NULL)
                {
                        free(line);
                        printf("Exiting interactive mode...\n");
                }
        }

        destroy_calculator(calculator);
        return 0;
}

/* shifts and replace $id with the actual value
 * this is an example of using the history, is not part of the main library
 * */
char *replace_id_with_value(struct Calculator *handler, char *line)
{
        char *pos = NULL;
        size_t offset = 0;
        size_t len = 0;
        char *new_line = NULL;

        if ((pos = strchr(line, '$')) == NULL)
                return line;

        offset = pos - line;
        len = strlen(line);

        size_t buff_size = len * 2 + 1;

        new_line = malloc(sizeof(char) * buff_size);
        if (!new_line)
                return NULL;

        strcpy(new_line, line);

        pos = new_line + offset;

        while (pos)
        {
                size_t id;
                size_t tail_len;
                if (sscanf(pos + 1, "%zu", &id) == 1) 
                {
                        struct Expression *expr = get_history_by_id(handler, id);
                        if (expr) 
                        {
                                char value_str[50];
                                snprintf(value_str, sizeof(value_str), "%.2f", expr->result);
                                size_t value_len = strlen(value_str);
                                size_t id_len = snprintf(NULL, 0, "%zu", id);

                                /* Checks if the buffer size is enough for containing
                                 * the string with the replacements */
                                while (value_len + len - id_len + 1 >= buff_size) {
                                        buff_size *= 2;
                                        size_t offset_pos = pos - new_line;
                                        new_line = realloc(new_line, buff_size);
                                        if (!new_line) {
                                                perror("Failed to realloc memory");
                                                exit(EXIT_FAILURE);
                                        }
                                        pos = new_line + offset_pos;
                                }

                                /* Shifts characters to the right to make space for the new value.
                                 * Specifically, move characters from the position after the variable
                                 * $id to the end of the string, to accommodate the new value.
                                 * After shifting, replace the $id placeholder with the actual value
                                 * */
                                tail_len = strlen(pos + id_len + 1);
                                memmove(pos + value_len, pos + id_len + 1, tail_len + 1);
                                memcpy(pos, value_str, value_len);
                                len += value_len - id_len - 1;
                                new_line[len] = '\0';
                                pos = pos + id_len + 1;
                        }
                        else 
                        {
                                printf("ERROR: variable $%zu not found\n", id);
                                return line;
                        }
                        free(expr->expr);
                        free(expr);
                }
                else 
                        return line;

                pos = strchr(pos, '$');
        }

        return new_line;
}
