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
        NUM,
        ADD_SUB,
        MUL_DIV,
        MAX,
};

struct Token
{
        char *val;
        enum Type type;
        enum Bp bp;
};

struct Tokens
{
        struct Token *tokens;
        size_t len;
};

extern struct Tokens *tokens;
extern char *input;
extern size_t input_len;

void    *calc_malloc(size_t len);
void    calc_log(char *message, const char *function, int line);
void    calc_cleanup();
void    add_token(struct Tokens *tokens, char *str, enum Type type, enum Bp bp);
void    debug_tokens(struct Tokens *tokens);

#endif
