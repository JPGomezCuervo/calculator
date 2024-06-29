#ifndef CALC_H
#define CALC_H
#include <stddef.h>
#include <stdbool.h>

enum Type
{
        OP_ADD,
        OP_SUB,
        OP_MUL,
        OP_DIV,
        OPEN_PARENTHESIS,
        CLOSE_PARENTHESIS,
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
void    *calc_calloc(int num, size_t size);
void    calc_log(char *message, const char *function, int line);
void    calc_cleanup();
void    add_token(char *str, enum Type type, enum Bp bp);
void    debug_tokens(struct Lexer *tokens);
struct  Token *next();
struct  Token *peek();
void    free_tree(struct Leaf *tree);
bool    is_operator(enum Type t);
bool    is_parenthesis(enum Type t);
enum    Type get_type(char c);
enum    Bp get_bp(char c);

#endif
