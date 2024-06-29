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

//TODO: Define error codes
//TODO: Handle parenthesis in tree
//TODO: Handle errors when input empty
//TODO: Check errors before entry to the functions

struct Leaf *increasing_prec(enum Bp min_bp);
struct Leaf *parse_leaf();
struct Leaf *make_leaf(struct Token *tk);
void debug_tree(struct Leaf *leaf, const char *indent);
float eval(struct Leaf *tree);
void handle_number(bool *was_number, char *temp, int *pos, char c);
void handle_number_end(bool *was_number, char *temp, int *pos);

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
                        if (i == 0 && is_operator(t)) 
                        {
                                temp[0] = c;
                                add_token(temp, t, bp);
                                continue;
                        }

                        /* Store char into a buffer until another operator is found */
                        if (isdigit(c))
                                handle_number(&was_number, temp, &pos, c);

                        if (is_operator(t) || is_parenthesis(t))
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
        tree = increasing_prec(MIN_LIMIT);

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
        // TODO: Handle parenthesis, numbers and unknown
        struct Token *tk = next();

        if (is_operator(tk->type)) 
                return make_leaf(tk);

        return make_leaf(tk);

}

struct Leaf *increasing_prec(enum Bp min_bp)
{
        struct Leaf *left = parse_leaf();
        struct Token *next_t = NULL;
        next_t = next();

        if (!next_t)
                return left;

        if (is_operator(next_t->type))
        {
                while (next_t->bp > min_bp) 
                {
                        struct Token *op = next_t;
                        struct Leaf *right = increasing_prec(op->bp);

                        left = make_binary_expr(op,left, right);
                        next_t = peek();

                        if (!next_t)
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

float eval(struct Leaf *tree) {
    if (tree == NULL) {
        fprintf(stderr, "Error: Null tree node encountered\n");
        exit(EXIT_FAILURE);
    }

    if (tree->value == NULL) {
        fprintf(stderr, "Error: Tree node has no value\n");
        exit(EXIT_FAILURE);
    }

    if (tree->value->type == NUMBER) {
        char *endptr;
        return strtof(tree->value->val, &endptr);
    }

    float lhs = 0.0, rhs = 0.0;

    if (tree->left) {
        lhs = eval(tree->left);
    } else {
        fprintf(stderr, "Error: Left subtree is NULL\n");
        exit(EXIT_FAILURE);
    }

    if (tree->right) {
        rhs = eval(tree->right);
    } else {
        fprintf(stderr, "Error: Right subtree is NULL\n");
        exit(EXIT_FAILURE);
    }

    switch (tree->value->type) {
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
        case UNKNOWN:
        default:
            fprintf(stderr, "Error: Unknown operator %s\n", tree->value->val);
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
