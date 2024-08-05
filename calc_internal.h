#ifndef CALC_INTERNAL_H
#define CALC_INTERNAL_H
#include <assert.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "calc.h"

#define DELIMITER '?'

#define ERR_NO_ERR 0
#define ERR_NO_INPUT 1
#define ERR_DIVIDE_BY_ZERO 2
#define ERR_UNKNOWN_OPERATOR 3
#define ERR_SYNTAX 4
#define ERR_HEAP_ALLOC 5

#define TokenType_UNKNOWN      0
#define TokenType_UNARY_NEG    1
#define TokenType_UNARY_POS    2
#define TokenType_OP_ADD       3
#define TokenType_OP_SUB       4
#define TokenType_OP_MUL       5
#define TokenType_OP_DIV       6
#define TokenType_OPEN_PARENT  7
#define TokenType_CLOSE_PARENT 8
#define TokenType_LIMIT        9
#define TokenType_NUMBER      10

#define BP_UNKNOWN      0
#define BP_MIN_LIMIT    1
#define BP_NUMBER       2
#define BP_ADD_SUB      3
#define BP_MUL_DIV      4
#define BP_MAX          5

typedef uint8_t token_type;
typedef uint8_t error_code;
typedef uint8_t binary_power;

struct Calculator_heap
{
        uint8_t *next_heap;
};

union Value
{
        char sign[8];
        double number;
};

struct Data {
        bool is_number;
        union Value val;
};

struct Lexer {
        struct Data **data;
        size_t len;
        size_t curr;
};

struct Leaf {
        struct Data data;
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
void free_tree(struct Calculator *handler, struct Leaf *tree);
int make_tokens(struct Calculator *handler);
void add_token(struct Calculator *handler, size_t *i, token_type t);
struct Data *get_next(struct Calculator *handler);
struct Data peek(struct Calculator *handler);
token_type get_type(struct Data c);
token_type get_bp(struct Data c);
bool is_operator(token_type t);
bool is_parenthesis(token_type t);
bool is_number(char c);
struct Leaf *get_free_leaf(Calculator *handler);
void recycle_leaf(struct Calculator *handler, struct Leaf *p_leaf);
struct Leaf *parse_expr(struct Calculator *handler, binary_power bp);
struct Leaf *increasing_prec(struct Calculator *handler, struct Leaf *left, binary_power min_bp);
struct Leaf *parse_leaf(struct Calculator *handler);
struct Leaf *make_leaf(struct Calculator *handler, struct Data *tk);
struct Leaf *make_binary_expr(struct Calculator *handler, struct Data *op, struct Leaf *left, struct Leaf *right);
double eval_tree(Calculator *handler, struct Leaf *tree);
void check_semantics(struct Calculator *handler);
struct Lexer *initialize_tokens(struct Calculator *handler);
void dead(Calculator *handler, error_code err);
void print_tree(struct Leaf *leaf, const char *indent);
void debug_tokens(struct Calculator *handler);
void debug_tree(struct Calculator *handler);
struct Expression *copy_expression(const struct Expression *src);

#endif
