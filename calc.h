#ifndef CALC_H
#define CALC_H
#include <stddef.h>

enum Calc_err {
    ERR_NO_ERR,
    ERR_NO_INPUT,
    ERR_DIVIDE_BY_ZERO,
    ERR_UNKNOWN_OPERATOR,
    ERR_SYNTAX,
};

typedef struct Calculator Calculator; 
typedef struct Expression {
    int id;
    char *expr;
    double result;
} Expression;

struct Calculator *init_calculator(size_t history_size);
double calculate_expr(struct Calculator *handler, char *str);
void destroy_calculator(Calculator *handler);
struct Expression **get_history(struct Calculator *handler);
size_t get_history_len(struct Calculator *handler);
enum Calc_err get_error_code(Calculator *handler);
char *error_message(Calculator *handler);

#endif
