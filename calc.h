#ifndef CALC_H
#define CALC_H
#include <stddef.h>
#include <stdbool.h>

#define DELIMITER '?'

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

void    *calc_malloc(size_t len);
void    *calc_calloc(int num, size_t size);
void    *calc_realloc(void *p, size_t new_size);
void    calc_log(char *message, const char *function, int line);
void    calc_cleanup();
void    add_token(struct Lexer *tokens, const char *input, size_t *i, enum Type t, size_t input_len, size_t tokens_pos);
void    debug_tokens(struct Lexer *tokens);
char    *get_next();
char    peek();
void    free_tree(struct Leaf *tree);
bool    is_operator(enum Type t);
enum    Type get_type(char c);
enum    Bp get_bp(char c);
bool    is_parenthesis(enum Type t);
void    handle_number(bool *was_number, char *temp, int *pos, char c);
void    handle_number_end(bool *was_number, char *temp, int *pos);
void    debug_tree(struct Leaf *leaf, const char *indent);
struct  Leaf *increasing_prec(struct Leaf *left, enum Bp min_bp);
struct  Leaf *parse_leaf();
struct Leaf *make_leaf(char *tk);
float   eval_tree(struct Leaf *tree);
struct  Leaf *parse_expr(enum Bp bp);

#endif
