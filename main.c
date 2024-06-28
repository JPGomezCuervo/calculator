#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "calc.h"

#define BUFF_SIZE 102
#define DELIMITER '?'
#define TEMP_STR 51

struct Lexer *tokens = NULL;  
struct Leaf *tree = NULL;
char *input = {0};           
size_t input_len = 0; 

//TODO: Error parsing the PARENTHESIS in this expr'12+2*3(7)'
//TODO: Define error codes
//TODO: Find valgrind error
//TODO: Handle parenthesis in tree

struct Leaf *increasing_prec(enum Bp min_bp);
struct Leaf *parse_leaf();
struct Token *next();
struct Token *peek();
struct Leaf *make_leaf(struct Token *tk);
void debug_tree(struct Leaf *leaf, const char *indent);

int main(int argsc, char **argsv)
{
        atexit(calc_cleanup);


        tokens = calc_malloc(sizeof(struct Lexer));

        input = calc_malloc(sizeof(char) * BUFF_SIZE);


        if (argsc < 2)
        {
                printf("USE: calc <expr>\n");
                exit(1);
        }



        // Joins all args into one string
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



        tokens->len = 0;
        tokens->tokens = calc_malloc(sizeof(struct Token) * BUFF_SIZE);


        {
                char temp[TEMP_STR];
                int pos = 0;
                bool was_number = false;

                temp[TEMP_STR - 1] = '\0';

                for (size_t i = 0; i < input_len; i++) 
                {
                        char c = input[i];
                        enum Type t;
                        enum Bp bp;

                        switch (c)
                        {
                                case '(':
                                case ')':
                                        t = PARENTHESIS;
                                        bp = MAX;
                                        if (i == 0)
                                        {
                                                temp[0] = c;
                                                temp[1] = '\0';
                                                add_token(tokens, temp, t, bp);
                                                continue;
                                        }

                                        break;
                                case '+':
                                case '-':
                                        t = OPERATOR;
                                        bp = ADD_SUB;
                                        if (i == 0)
                                        {
                                                temp[0] = c;
                                                temp[1] = '\0';
                                                add_token(tokens, temp, t, bp);
                                                continue;
                                        }

                                        break;
                                case '*':
                                case '/':
                                        t = OPERATOR;
                                        bp = MUL_DIV;

                                        break;
                                case '.':
                                case '0':
                                case '1':
                                case '2':
                                case '3':
                                case '4':
                                case '5':
                                case '6':
                                case '7':
                                case '8':
                                case '9':
                                        was_number = true;
                                        t = NUMBER;
                                        temp[pos] = c;
                                        pos++;

                                        continue;
                                case '?':
                                        temp[pos] = '\0';
                                        add_token(tokens, temp, NUMBER, NUM);
                                        break;
                                default:
                                        t = UNKNOWN;
                                        bp = UNKNOWNBP;
                                        break;
                        }

                        if (was_number && (t == OPERATOR || t == PARENTHESIS))
                        {
                                temp[pos] = '\0';
                                add_token(tokens, temp, NUMBER, NUM);
                                was_number = false;

                                temp[pos] = c;
                                temp[pos+1] = '\0';
                                add_token(tokens, &temp[pos], t, bp);
                                pos = 0;
                        }
                }

        }

        debug_tokens(tokens);
        tree = increasing_prec(MIN_LIMIT);
        debug_tree(tree, "");

        return 0;
}

struct Token *next()
{
        struct Token *ptk = NULL;

        if (tokens->curr < tokens->len)
        {
                ptk = &tokens->tokens[tokens->curr];
                tokens->curr++;
        }

        return ptk; 
}

struct Token *peek()
{
        struct Token *ptk = NULL;

        if (tokens->curr < tokens->len)
        {
                ptk = &tokens->tokens[tokens->curr + 1];
        }

        return ptk; 
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
        struct Token *tk = next();
        
        switch (tk->type)
        {
                case OPERATOR:
                        return make_leaf(tk);
                case NUMBER:
                        return make_leaf(tk);
                case PARENTHESIS:
                case UNKNOWN:
                case LIMIT:
                default:
                                return NULL;
        }
}

struct Leaf *increasing_prec(enum Bp min_bp)
{
        struct Leaf *left = parse_leaf();
        struct Token *next_t = next();

        if (!next_t)
                return left;

        if (next_t->type == OPERATOR)
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

    // Print current node
    printf("%sHead: %s\n", indent, leaf->value ? leaf->value->val : "NULL");
    
    // Print left child
    if (leaf->left)
    {
        printf("%sLeft:\n", indent);
        char new_indent[256];
        snprintf(new_indent, sizeof(new_indent), "%s    ", indent);
        debug_tree(leaf->left, new_indent);
    }

    // Print right child
    if (leaf->right)
    {
        printf("%sRight:\n", indent);
        char new_indent[256];
        snprintf(new_indent, sizeof(new_indent), "%s    ", indent);
        debug_tree(leaf->right, new_indent);
    }
}
