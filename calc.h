#ifndef CALC_H
#define CALC_H
#include <stddef.h>

enum Type
{
        OPERATOR,
        NUMBER,
        PARENTHESIS,
        UNKNOWN,
        LIMIT,
};

enum Bp
{
        MIN_LIMIT,
        NUM,
        ADD_SUB,
        MUL_DIV,
        MAX,
        UNKNOWNBP,
};

struct Token
{
        char *val;
        enum Type type;
        enum Bp bp;
};

struct Lexer
{
        struct Token *tokens;
        size_t len;
        size_t curr;
};

struct Leaf
{
        struct Token *value;
        struct Leaf *left;
        struct Leaf *right;
};

extern struct Lexer *tokens;
extern char *input;
extern size_t input_len;
extern struct Leaf *tree;

void    *calc_malloc(size_t len);
void    calc_log(char *message, const char *function, int line);
void    calc_cleanup();
void    add_token(struct Lexer *tokens, char *str, enum Type type, enum Bp bp);
void    debug_tokens(struct Lexer *tokens);
void    free_tree(struct Leaf *tree);

#endif