#ifndef CALC_INTERNAL_H
#define CALC_INTERNAL_H
#include <assert.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "calc.h"

#define DELIMITER '?'

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

// enum Type {
//     UNARY_NEG,
//     UNARY_POS,
//     OP_ADD,
//     OP_SUB,
//     OP_MUL,
//     OP_DIV,
//     OPEN_PARENT,
//     CLOSE_PARENT,
//     LIMIT,
//     NUMBER,
//     UNKNOWN
// };

#define BP_UNKNOWN      0
#define BP_MIN_LIMIT    1
#define BP_NUMBER       2
#define BP_ADD_SUB      3
#define BP_MUL_DIV      4
#define BP_MAX          5

// enum Bp {
//     MIN_LIMIT,
//     BP_NUMBER,
//     BP_ADD_SUB,
//     BP_MUL_DIV,
//     BP_MAX,
//     BP_UNKNOWN,
// };

struct Lexer {
    char **chars;
    size_t len;
    size_t curr;
};

#pragma pack(push, 1)
union LeafData // 8 bytes
{
        unsigned char value[8];
        double number;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct Leaf {
        struct Leaf *left;   // 8 bytes
        struct Leaf *right;  // 8 bytes
        union LeafData data; // 8 bytes
};
#pragma pack(pop)

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
void add_token(struct Calculator *handler, size_t *i, unsigned char token_type, size_t tokens_pos);
char *get_next(struct Calculator *handler);
char peek(struct Calculator *handler);
unsigned char get_type(char c);
unsigned char get_bp(char c);
bool is_operator(unsigned char token_type);
bool is_parenthesis(unsigned char token_type);
bool is_number(char c);
struct Leaf *parse_expr(struct Calculator *handler, unsigned char bp);
struct Leaf *increasing_prec(struct Calculator *handler, struct Leaf *left, unsigned char min_bp);
struct Leaf *parse_leaf(struct Calculator *handler);
struct Leaf *make_leaf(struct Calculator *handler, char *tk);
struct Leaf *make_binary_expr(char *op, struct Leaf *left, struct Leaf *right);
double eval_tree(Calculator *handler, struct Leaf *tree);
void check_semantics(struct Calculator *handler);
struct Lexer *initialize_tokens(struct Calculator *handler);
void dead(Calculator *handler, enum Calc_err err);
void print_tree(struct Leaf *leaf, const char *indent);
void debug_tokens(struct Calculator *handler);
void debug_tree(struct Calculator *handler);

#endif
