#ifndef CALC_INTERNAL_H
#define CALC_INTERNAL_H
#include <assert.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "calc.h"

#define DELIMITER '?'

enum Type {
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

enum Bp {
    MIN_LIMIT,
    BP_NUMBER,
    BP_ADD_SUB,
    BP_MUL_DIV,
    BP_MAX,
    BP_UNKNOWN,
};

struct Lexer {
    char **chars;
    size_t len;
    size_t curr;
};

struct Leaf {
    char *value;
    struct Leaf *left;
    struct Leaf *right;
};

struct History {
    struct Expression **exprs;
    size_t len;
    size_t capacity;
};

void *calc_malloc(size_t len);
void *calc_calloc(int num, size_t size);
void *calc_realloc(void *p, size_t new_size);
void calc_log(char *message, const char *function, int line);
void calc_cleanup(struct Calculator *handler);
void free_tree(struct Leaf *tree);
int make_tokens(struct Calculator *handler);
void add_token(struct Calculator *handler, size_t *i, enum Type t, size_t tokens_pos);
char *get_next(struct Calculator *handler);
char peek(struct Calculator *handler);
enum Type get_type(char c);
enum Bp get_bp(char c);
bool is_operator(enum Type t);
bool is_parenthesis(enum Type t);
bool is_number(char c);
struct Leaf *parse_expr(struct Calculator *handler, enum Bp bp);
struct Leaf *increasing_prec(struct Calculator *handler, struct Leaf *left, enum Bp min_bp);
struct Leaf *parse_leaf(struct Calculator *handler);
struct Leaf *make_leaf(char *tk);
struct Leaf *make_binary_expr(char *op, struct Leaf *left, struct Leaf *right);
double eval_tree(Calculator *handler, struct Leaf *tree);
void check_semantics(struct Calculator *handler);
struct Lexer *initialize_tokens(struct Calculator *handler);
void dead(Calculator *handler, enum Calc_err err);
enum Calc_err error_code(Calculator *handler);
char *error_message(Calculator *handler);
void print_tree(struct Leaf *leaf, const char *indent);

#endif // CALC_INTERNAL_H
