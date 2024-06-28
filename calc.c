#include "calc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void add_token(struct Lexer *tokens, char *str, enum Type type, enum Bp bp)
{
    struct Token tk;
    // TODO: handle errors and numbers

    if (is_operator(type))
    {
            tk.val = calc_calloc(2, sizeof(char));
            tk.val[0] = str[0];
            tk.val[1] = '\0';
            tk.bp = bp;
            tk.type = type;
            tokens->tokens[tokens->len] = tk;
            tokens->len++;
    }
    else
    {
            tk.val = calc_calloc(strlen(str) + 1, sizeof(char));
            strcpy(tk.val, str);
            tk.type = NUMBER;
            tk.bp = NUM;
            tokens->tokens[tokens->len] = tk;
            tokens->len++;
    }
}

struct Token *next()
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
                return &tokens->tokens[tokens->curr + 1];
        }
        return NULL;
}

void debug_tokens(struct Lexer *tokens)
{
    const char *lookup_t[] = {
        "OP_ADD",
        "OP_SUB",
        "OP_MUL",
        "OP_DIV",
        "OPEN_PARENTHESIS",
        "CLOSE_PARENTHESIS",
        "LIMIT",
        "NUMBER",
        "UNKNOWN"
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

bool is_operator(enum Type t)
{
        switch (t)
        {
                case OP_ADD:
                case OP_SUB:
                case OP_MUL:
                case OP_DIV:
                        return true;
                case OPEN_PARENTHESIS:
                case CLOSE_PARENTHESIS:
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
        if (t == OPEN_PARENTHESIS || t == CLOSE_PARENTHESIS)
                return true;

        return false;
}
