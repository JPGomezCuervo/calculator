#include "calc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

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

void *calc_realloc(void *p, size_t new_size)
{
    p = realloc(p, new_size);

    if (!p) {
        calc_log("Error in reallocation", __func__, __LINE__);
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
                        free(tokens->chars[i]);

                free(tokens);
        }


        if (input) 
                free(input);

        if (tree)
                free_tree(tree);
}

void add_token(struct Lexer *tokens, const char *input, size_t *i, enum Type t, size_t input_len, size_t tokens_pos) 
{

        if (t == NUMBER)
        {
                size_t j = 0;

                char *str = calc_malloc((input_len - tokens_pos + 1) * sizeof(char));

                while (isdigit(input[*i]) && *i < input_len) 
                {
                        str[j] = input[*i];
                        j++;
                        (*i)++;
                }

                str[j] = '\0';
                str = calc_realloc(str, sizeof(char) * j);

                tokens->chars[tokens_pos] = str;
                (*i)--;  // Adjust index since the for loop will increment i
        }
        else
        {

                tokens->chars[tokens_pos] = calc_malloc(2 * sizeof(char));
                tokens->chars[tokens_pos][0] = input[*i];
                tokens->chars[tokens_pos][1] = '\0';
        }
}

char *get_next()
{
        assert(tokens != NULL);

        char *pc = NULL;

        if (tokens->curr != DELIMITER)
        {
                pc = tokens->chars[tokens->curr];
                tokens->curr++;
        }

        return pc;
}

char *peek()
{
        assert(tokens != NULL);

        if (tokens->curr != DELIMITER)
                return tokens->chars[tokens->curr];

        return NULL;
}

void debug_tokens(struct Lexer *tokens)
{
    const char *lookup_t[] = {
        "UNARY_NEG",
        "UNARY_POS",
        "OP_ADD",
        "OP_SUB",
        "OP_MUL",
        "OP_DIV",
        "OPEN_PARENTHESIS",
        "CLOSE_PARENTHESIS",
        "LIMIT",
        "NUMBER",
        "UNARY",
        "UNKNOWN"
};
    for (size_t i = 0; i < tokens->len; i++)
    {
        printf("index: %zu, Value: %s, Type: %s, Precedence: %d\n", 
                i, 
                &tokens->chars[i][0],
                lookup_t[get_type(tokens->chars[i][0])],
                get_bp(tokens->chars[i][0])
              );
    }
}

enum Type get_type(char c) 
{

        switch (c) 
        {
                case '+':
                        return OP_ADD;
                case '-':
                        return OP_SUB;
                case '*':
                        return OP_MUL;
                case '/':
                        return OP_DIV;
                case '(':
                        return OPEN_PARENT;
                case ')':
                        return CLOSE_PARENT;
                case '?':
                        return LIMIT;
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                        return NUMBER;
                default:
                        return UNKNOWN;
        }
}

enum Bp get_bp(char c) 
{

        switch (c) 
        {
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

bool is_operator(enum Type t)
{
        switch (t)
        {
                case OP_ADD:
                case OP_SUB:
                case OP_MUL:
                case OP_DIV:
                        return true;
                case OPEN_PARENT:
                case CLOSE_PARENT:
                case LIMIT:
                case NUMBER:
                case UNKNOWN:
                        return false;
                default:
                        return false;
        }
}

bool is_parenthesis(enum Type t)
{
        return (t == OPEN_PARENT || t == CLOSE_PARENT);
}

void debug_tree(struct Leaf *leaf, const char *indent)
{
        if (leaf == NULL)
                return;

        printf("%sHead: %s\n", indent, leaf->value ? leaf->value : "NULL");

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

struct Leaf *make_leaf(char *tk)
{
        struct Leaf *leaf = calc_malloc(sizeof(struct Leaf));
        leaf->value = tk;
        leaf->left = NULL;
        leaf->right = NULL;
        return leaf;
}
struct Leaf *make_binary_expr(char *op, struct Leaf *left, struct Leaf *right)
{
        struct Leaf *leaf = calc_malloc(sizeof(struct Leaf));
        leaf->value = op;
        leaf->left = left;
        leaf->right = right;

        return leaf;
}

struct Leaf *parse_leaf()
{
        char *tk = get_next();
        struct Leaf *leaf = NULL;
        enum Type t = get_type(*tk);

        if (t == OP_ADD || t == OP_SUB)
        {
                struct Leaf *right = parse_leaf(); 
                leaf = make_leaf(tk);
                leaf->right = right;
        }
        else if (t == OPEN_PARENT)
        {
                leaf = parse_expr(MIN_LIMIT);
                /* consumes close parenthesis */
                get_next(); 
        }
        else 
        {
                leaf = make_leaf(tk);
        }
                
        return leaf;
}

struct Leaf *parse_expr(enum Bp bp)
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
        char *next = peek();
        enum Type t = get_type(*next);
        enum Bp bp = get_bp(*next);


        if (*next == DELIMITER || t == CLOSE_PARENT)
                return left;

        if (is_operator(t))
        {
                while (bp >= min_bp) 
                {
                        char *op = get_next();
                        struct Leaf *right = parse_expr(bp);
                        left = make_binary_expr(op,left, right);

                        next = peek();
                        t = get_type(*next);

                        if (*next == DELIMITER || t == CLOSE_PARENT)
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
        enum Type t = get_type(*tree->value);

        if (t == NUMBER) 
                return strtof(tree->value, NULL);


        if (tree->left == NULL)
                lhs = 0;
        else
                lhs = eval_tree(tree->left);


        if (tree->right == NULL)
                rhs = 0;
        else
                rhs = eval_tree(tree->right);


        switch (t) 
        {
                case OP_ADD:
                        return lhs + rhs;
                case OP_SUB:
                        return lhs - rhs;
                case OP_MUL:
                        return lhs * rhs;
                case OP_DIV:
                        if (rhs == 0) {
                                fprintf(stderr, "Error: Division by zero\n");
                                exit(EXIT_FAILURE);
                        }
                        return lhs / rhs;
                case OPEN_PARENT: 
                case CLOSE_PARENT: 
                case LIMIT: 
                case NUMBER:
                        fprintf(stderr, "Error: invalid operator %s\n", tree->value);
                        exit(EXIT_FAILURE);
                case UNARY_NEG:
                        return -rhs;
                default:
                        fprintf(stderr, "Error: unknown operator %s\n", tree->value);
                        exit(EXIT_FAILURE);
        }
}
