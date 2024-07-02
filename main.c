#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "calc.h"

#define BUFF_SIZE 101
#define DELIMITER '?'
#define TEMP_STR 51

struct Lexer *tokens = NULL;  
struct Leaf *tree = NULL;
char *input = NULL;           
size_t input_len = 0; 

//TODO: fix unary/prefix
//TODO: Define error codes
//TODO: Handle parenthesis in tree
//TODO: Handle errors when input empty
//TODO: Check errors before entry to the functions

struct Leaf *increasing_prec(struct Leaf *left, enum Bp min_bp);
struct Leaf *parse_leaf();
struct Leaf *make_leaf(struct Token *tk);
void debug_tree(struct Leaf *leaf, const char *indent);
float eval(struct Leaf *tree);
void handle_number(bool *was_number, char *temp, int *pos, char c);
void handle_number_end(bool *was_number, char *temp, int *pos);
struct Leaf *parse_expr(enum Bp bp);

int main(int argsc, char **argsv)
{
        atexit(calc_cleanup);

        tokens = calc_malloc(sizeof(struct Lexer));
        tokens->len = 0;
        tokens->tokens = calc_calloc(BUFF_SIZE, sizeof(struct Token));

        input = calc_calloc(BUFF_SIZE, sizeof(char));


        if (argsc < 2)
        {
                printf("USE: calc <expr>\n");
                exit(1);
        }


        /* Joins all args into one string */
        if (argsc > 1)
        {
                for (int i = 1; i < argsc; i++)
                {
                        for (size_t j = 0; j < strlen(argsv[i]); j++) 
                        {
                                if (!isspace(argsv[i][j]))
                                        strncat(input, &argsv[i][j], 1);
                        }
                }
        } 

        input_len = strlen(input);
        input[input_len] = (char) DELIMITER;
        input[++input_len] = '\0';


        {
                char temp[TEMP_STR];
                temp[TEMP_STR - 1] = '\0';
                int pos = 0;
                bool was_number = false;


                for (size_t i = 0; i < input_len; i++) 
                {
                        char c = input[i];
                        enum Type t = get_type(c);
                        enum Bp bp = get_bp(c);

                        /* Handle prefix */
                        if (i == 0 && (is_operator(t) || t == OPEN_PARENT)) 
                        {
                                if (c == '-')
                                        t = UNARY_NEG;

                                if (c == '+')
                                        t = UNARY_POS;

                                temp[0] = c;
                                add_token(temp, t, bp);
                                continue;
                        }

                        /* Store char into a buffer until another operator is found */
                        if (isdigit(c))
                                handle_number(&was_number, temp, &pos, c);

                        if (is_operator(t) || t == OPEN_PARENT || t == CLOSE_PARENT)
                        {
                                if (was_number)
                                        handle_number_end(&was_number, temp, &pos);

                                temp[0] = c;
                                add_token(temp, t, bp);
                        }
                        if (t == LIMIT)
                        {
                                if (was_number)
                                {
                                        temp[pos] = '\0';
                                        handle_number_end(&was_number, temp, &pos);
                                }
                        }
                }

        }

        debug_tokens(tokens);
        tree = parse_expr(MIN_LIMIT);

        if (tree != NULL)
        {
                debug_tree(tree, "");
                printf("result: %.2f\n", eval(tree));
        }

        return 0;
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

        if (tk->type == UNARY_NEG)
        {
                struct Leaf *right = parse_leaf(); 
                leaf = make_leaf(tk);
                leaf->right = right;
        }
        else if (tk->type == OPEN_PARENT)
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

float eval(struct Leaf *tree)
{
        assert(tree != NULL);
        assert(tree->value != NULL);

        float lhs = 0.0, rhs = 0.0;

        if (tree->value->type == NUMBER) 
                return strtof(tree->value->val, NULL);


        if (tree->left == NULL)
                lhs = 0;
        else
                lhs = eval(tree->left);


        if (tree->right == NULL)
                rhs = 0;
        else
                rhs = eval(tree->right);


        switch (tree->value->type) 
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
                        fprintf(stderr, "Error: invalid operator %s\n", tree->value->val);
                        exit(EXIT_FAILURE);
                case UNARY_NEG:
                        return -rhs;
                default:
                        fprintf(stderr, "Error: unknown operator %s\n", tree->value->val);
                        exit(EXIT_FAILURE);
        }
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
