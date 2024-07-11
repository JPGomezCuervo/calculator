#ifndef CALC_H
#define CALC_H
#include <assert.h>
#include <stddef.h>
#include <stdbool.h>

#define DELIMITER '?'

/* error codes */

enum Calc_err
{
        ERR_NO_INPUT = 1,
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

extern  struct Lexer *tokens;
extern  char *input;
extern  size_t input_len;
extern  struct Leaf *tree;
extern bool continuous_mode;

void    *calc_malloc(size_t len);
void    *calc_calloc(int num, size_t size);
void    *calc_realloc(void *p, size_t new_size);
void    calc_log(char *message, const char *function, int line);
void    calc_cleanup();
void    free_tree(struct Leaf *tree);
void    handle_number(bool *was_number, char *temp, int *pos, char c);
void    handle_number_end(bool *was_number, char *temp, int *pos);
void    add_token(size_t *i, enum Type t, size_t input_len, size_t tokens_pos);
void    debug_tokens(struct Lexer *tokens);
char    *get_next();
char    peek();
bool    is_operator(enum Type t);
bool    is_parenthesis(enum Type t);
enum    Type get_type(char c);
enum    Bp get_bp(char c);
struct  Leaf *parse_expr(enum Bp bp);
void    debug_tree(struct Leaf *leaf, const char *indent);
struct  Leaf *increasing_prec(struct Leaf *left, enum Bp min_bp);
struct  Leaf *parse_leaf();
struct  Leaf *make_leaf(char *tk);
double   eval_tree(struct Leaf *tree);
void    dead(enum Calc_err err);
bool    check_semantics();
int     calc_scan();
int     make_tokens();
struct Lexer *initialize_tokens(size_t input_len);

#endif
