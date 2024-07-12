#ifndef CALC_H
#define CALC_H
#include <assert.h>
#include <stddef.h>
#include <stdbool.h>

#define DELIMITER '?'

/* error codes */

enum Calc_err
{
        ERR_NO_INPUT,
        ERR_DIVIDE_BY_ZERO,
        ERR_UNKNOWN_OPERATOR,
        ERR_SYNTAX,
};

enum Type
{
        UNARY_NEG,
        UNARY_POS,
        OP_ADD,
        OP_SUB,
        OP_MUL,
        OP_DIV,
        OPEN_PARENT,
        CLOSE_PARENT,
        LIMIT,
        NUMBER,
        UNKNOWN
};

enum Bp
{
        MIN_LIMIT,
        BP_NUMBER,
        BP_ADD_SUB,
        BP_MUL_DIV,
        BP_MAX,
        BP_UNKNOWN,
};

struct Lexer
{
        char **chars;
        size_t len;
        size_t curr;
};

struct Leaf
{
        char *value;
        struct Leaf *left;
        struct Leaf *right;
};

/* opaque pointer */
typedef struct Calculator Calculator; 

void    *calc_malloc(size_t len);
void    *calc_calloc(int num, size_t size);
void    *calc_realloc(void *p, size_t new_size);
void    calc_log(char *message, const char *function, int line);
void calc_cleanup(struct Calculator *handler);
void    free_tree(struct Leaf *tree);
void    handle_number(bool *was_number, char *temp, int *pos, char c);
void    handle_number_end(bool *was_number, char *temp, int *pos);
void add_token(struct Calculator *handler, size_t *i, enum Type t, size_t tokens_pos);
void    debug_tokens(struct Lexer *tokens);
char *get_next(struct Calculator *handler);
char peek(struct Calculator *handler);
bool    is_operator(enum Type t);
bool    is_parenthesis(enum Type t);
enum    Type get_type(char c);
enum    Bp get_bp(char c);
struct Leaf *parse_expr(struct Calculator *handler, enum Bp bp);
void    debug_tree(struct Leaf *leaf, const char *indent);
struct Leaf *increasing_prec(struct Calculator *handler,struct Leaf *left, enum Bp min_bp);
struct Leaf *parse_leaf(struct Calculator *handler);
struct  Leaf *make_leaf(char *tk);
double   eval_tree(struct Leaf *tree);
void    dead(enum Calc_err err);
void check_semantics(struct Calculator *handler);
int make_tokens(struct Calculator *handler);
struct Lexer *initialize_tokens(struct Calculator *handler);
double calculate_expr(struct Calculator *handler, char *str);
struct Calculator *init_calculator();

#endif
