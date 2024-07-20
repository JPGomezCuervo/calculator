#include "calc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void *calc_malloc(size_t len)
{
    void *p = NULL;
    p = malloc(len);

    if (!p) {
        calc_log("Error in allocation", __func__, __LINE__);
        exit(EXIT_FAILURE);
    }

    memset(p, 0, len); 
    return p;
}

void *calc_calloc(int num, size_t size)
{
    void *p = NULL;
    p = calloc(num, size);

    if (!p) {
        calc_log("Error in allocation", __func__, __LINE__);
        exit(EXIT_FAILURE);
    }

    return p;
}

void calc_log(char *message, const char *function, int line)
{
        printf("%s at %s::line %d", message, function, line);
}

void free_tree(struct Leaf *tree)
{
        if (!tree)
                return;
        free_tree(tree->right);
        free_tree(tree->left);

        free(tree);
}

void calc_cleanup()
{
        if (tokens) 
        {
                for (size_t i = 0; i < tokens->len; i++)
                        free(tokens->tokens[i].val);

                free(tokens->tokens);
                free(tokens);
        }


        if (input) 
                free(input);

        if (tree)
                free_tree(tree);
}

void add_token(char *str, unsigned char t, unsigned char bp)
{
    struct Token tk;

    if (is_operator(t) || is_parenthesis(t) || t == TokenType_UNARY_NEG || t == TokenType_UNARY_POS)
    {
            tk.U.op[0] = str[0];
            tk.U.op[1] = '\0';
            tk.bp = bp;
            tk.type = t;
            tokens->tokens[tokens->len] = tk;
            tokens->len++;
    }
    else 
    {
            tk.U.number = atof(str);
            tk.type = TokenType_NUMBER;
            tk.bp = BP_NUMBER;
            tokens->tokens[tokens->len] = tk;
            tokens->len++;
    }
}

struct Token *get_next()
{
        struct Token *ptk = NULL;

        if (tokens && tokens->curr < tokens->len)
        {
                ptk = &tokens->tokens[tokens->curr];
                tokens->curr++;
        }

        return ptk;
}

struct Token *peek()
{

        if (tokens->curr < tokens->len)
        {
                return &tokens->tokens[tokens->curr];
        }
        return NULL;
}

void handle_number(bool *was_number, char *temp, int *pos, char c)
{
        *was_number = true;
        temp[*pos] = c;
        *pos += 1;
}

void handle_number_end(bool *was_number, char *temp, int *pos)
{
        *was_number = false;
        temp[*pos] = '\0';
        add_token(temp, NUMBER, BP_NUMBER);
        *pos = 0;
}

void debug_tokens(struct Lexer *tokens)
{
    const char *lookup_t[] = {
        "TokenType_UNARY_NEG",
        "TokenType_UNARY_POS",
        "TokenType_OP_ADD",
        "TokenType_OP_SUB",
        "TokenType_OP_MUL",
        "TokenType_OP_DIV",
        "TokenType_OPEN_PARENTHESIS",
        "TokenType_CLOSE_PARENTHESIS",
        "TokenType_LIMIT",
        "TokenType_NUMBER",
        "TokenType_UNARY",
        "TokenType_UNKNOWN"
};
    for (size_t i = 0; i < tokens->len; i++)
    {
        printf("index: %zu, Value: %s, Type: %s, Precedence: %d\n", 
                i, 
                tokens->tokens[i].val,
                lookup_t[tokens->tokens[i].type],
                tokens->tokens[i].bp
              );
    }
}

unsigned char get_type(char c) {

        switch (c) {
                case '+':
                        return TokenType_OP_ADD;
                case '-':
                        return TokenType_OP_SUB;
                case '*':
                        return TokenType_OP_MUL;
                case '/':
                        return TokenType_OP_DIV;
                case '(':
                        return TokenType_OPEN_PARENT;
                case ')':
                        return TokenType_CLOSE_PARENT;
                case '?':
                        return TokenType_LIMIT;
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                        return TokenType_NUMBER;
                default:
                        return TokenType_UNKNOWN;
        }
}

enum Bp get_bp(char c) {

        switch (c) {
                case ')':
                case '(':
                        return MIN_LIMIT;
                case '+':
                case '-':
                        return BP_ADD_SUB;
                case '*':
                case '/':
                        return BP_MUL_DIV;
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                        return BP_NUMBER;
                default:
                        return BP_UNKNOWN;
        }
}

bool is_operator(unsigned char t)
{
        switch (t)
        {
                case TokenType_OP_ADD:
                case TokenType_OP_SUB:
                case TokenType_OP_MUL:
                case TokenType_OP_DIV:
                        return true;
                case TokenType_OPEN_PARENT:
                case TokenType_CLOSE_PARENT:
                case TokenType_LIMIT:
                case TokenType_NUMBER:
                case TokenType_UNKNOWN:
                        return false;
                default:
                        return false;
        }
}

bool is_parenthesis(unsigned char t)
{
        return (t == TokenType_OPEN_PARENT || t == TokenType_CLOSE_PARENT);
}

void debug_tree(struct Leaf *leaf, const char *indent)
{
        if (leaf == NULL)
        {
                return;
        }

        printf("%sHead: %s\n", indent, leaf->value ? leaf->value->val : "NULL");

        if (leaf->left)
        {
                printf("%sLeft:\n", indent);
                char new_indent[256];
                snprintf(new_indent, sizeof(new_indent), "%s    ", indent);
                debug_tree(leaf->left, new_indent);
        }

        if (leaf->right)
        {
                printf("%sRight:\n", indent);
                char new_indent[256];
                snprintf(new_indent, sizeof(new_indent), "%s    ", indent);
                debug_tree(leaf->right, new_indent);
        }
}

struct Leaf *make_leaf(struct Token *tk)
{
        struct Leaf *leaf = calc_malloc(sizeof(struct Leaf));
        leaf->value = tk;
        leaf->left = NULL;
        leaf->right = NULL;
        return leaf;
}
struct Leaf *make_binary_expr(struct Token *op, struct Leaf *left, struct Leaf *right)
{
        struct Leaf *leaf = calc_malloc(sizeof(struct Leaf));
        leaf->value = op;
        leaf->left = left;
        leaf->right = right;

        return leaf;
}

struct Leaf *parse_leaf()
{
        struct Token *tk = get_next();
        struct Leaf *leaf = NULL;

        if (tk->type == TokenType_UNARY_NEG)
        {
                struct Leaf *right = parse_leaf(); 
                leaf = make_leaf(tk);
                leaf->right = right;
        }
        else if (tk->type == TokenType_OPEN_PARENT)
        {
                leaf = parse_expr(BP_MIN_LIMIT);
                /* consumes close parenthesis */
                get_next(); 
        }
        else 
        {
                leaf = make_leaf(tk);
        }
                
        return leaf;
}

struct Leaf *parse_expr(unsigned char bp)
{
        struct Leaf *left = parse_leaf();
        struct Leaf *node = NULL;

        while (true)
        {
                node = increasing_prec(left, bp);

                if (left == node)
                        break;
                left = node;
        }
        return left;
}

struct Leaf *increasing_prec(struct Leaf *left, enum Bp min_bp)
{
        struct Token *next = NULL;
        next = peek();


        if (next == NULL || next->type == CLOSE_PARENT)
                return left;

        if (is_operator(next->type))
        {
                while (next->bp >= min_bp) 
                {
                        struct Token *op = get_next();
                        struct Leaf *right = parse_expr(op->bp);
                        left = make_binary_expr(op,left, right);

                        next = peek();

                        if (next == NULL || next->type == CLOSE_PARENT)
                                break;
                }
        }
        return left;
}

float eval_tree(struct Leaf *tree)
{
        assert(tree != NULL);
        assert(tree->value != NULL);

        float lhs = 0.0, rhs = 0.0;

        if (tree->value->type == NUMBER) 
                return strtof(tree->value->val, NULL);


        if (tree->left == NULL)
                lhs = 0;
        else
                lhs = eval_tree(tree->left);


        if (tree->right == NULL)
                rhs = 0;
        else
                rhs = eval_tree(tree->right);


        switch (tree->value->type) 
        {
                case TokenType_OP_ADD:
                        return lhs + rhs;
                case TokenType_OP_SUB:
                        return lhs - rhs;
                case TokenType_OP_MUL:
                        return lhs * rhs;
                case TokenType_OP_DIV:
                        if (rhs == 0) {
                                fprintf(stderr, "Error: Division by zero\n");
                                exit(EXIT_FAILURE);
                        }
                        return lhs / rhs;
                case TokenType_OPEN_PARENT: 
                case TokenType_CLOSE_PARENT: 
                case TokenType_LIMIT: 
                case TokenType_NUMBER:
                        fprintf(stderr, "Error: invalid operator %s\n", tree->value->val);
                        exit(EXIT_FAILURE);
                case TokenType_UNARY_NEG:
                        return -rhs;
                default:
                        fprintf(stderr, "Error: unknown operator %s\n", tree->value->val);
                        exit(EXIT_FAILURE);
        }
}
