#ifndef CALC_H
#define CALC_H
#include <stddef.h>
#include <stdbool.h>

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

// enum Type
// {
//         UNARY_NEG,
//         UNARY_POS,
//         OP_ADD,
//         OP_SUB,
//         OP_MUL,
//         OP_DIV,
//         OPEN_PARENT,
//         CLOSE_PARENT,
//         LIMIT,
//         NUMBER,
//         UNKNOWN
// };


#define BP_UNKNOWN      0
#define BP_MIN_LIMIT    1
#define BP_NUMBER       2
#define BP_ADD_SUB      3
#define BP_MUL_DIV      4
#define BP_MAX          5

// enum Bp
// {
//         MIN_LIMIT,
//         BP_NUMBER,
//         BP_ADD_SUB,
//         BP_MUL_DIV,
//         BP_MAX,
//         BP_UNKNOWN,
// };

#pragma pack(push, 1)
struct Token
{
        union U
		{
			char op[4];     // 4 bytes
			float number;   // 4 bytes
		};
        unsigned char type; // 1 bytes 
        unsigned char bp;   // 1 bytes
};
#pragma pack(pop)

struct Lexer
{
        struct Token *tokens;
        size_t len;
        size_t curr;
};

#pragma pack(push, 1)
struct Leaf
{
        struct Leaf *left;    // 8 bytes
        struct Leaf *right;   // 8 bytes
        struct Token value;   // 6 bytes
};
#pragma pack(pop)

extern struct Lexer *tokens;
extern char *input;
extern size_t input_len;
extern struct Leaf *tree;

void    *calc_malloc(size_t len);
void    *calc_calloc(int num, size_t size);
void    calc_log(char *message, const char *function, int line);
void    calc_cleanup();
void    add_token(char *str, unsigned char type, unsigned char bp);
void    debug_tokens(struct Lexer *tokens);
struct  Token *get_next();
struct  Token *peek();
void    free_tree(struct Leaf *tree);
bool    is_operator(unsigned char t);
unsigned char get_type(char c);
unsigned char get_bp(char c);
bool    is_parenthesis(unsigned char t);
void    handle_number(bool *was_number, char *temp, int *pos, char c);
void    handle_number_end(bool *was_number, char *temp, int *pos);
void    debug_tree(struct Leaf *leaf, const char *indent);
struct  Leaf *increasing_prec(struct Leaf *left, enum Bp min_bp);
struct  Leaf *parse_leaf();
struct  Leaf *make_leaf(struct Token *tk);
float   eval_tree(struct Leaf *tree);
struct  Leaf *parse_expr(unsigned char bp);

#endif
