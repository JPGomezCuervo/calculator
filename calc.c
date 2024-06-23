#include "calc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *calc_malloc(size_t len)
{
    void *p;

    if (!(p = malloc(len))) {
        calc_log("Error in allocation", __func__, __LINE__);
        exit(EXIT_FAILURE);
    }

    return p;
}

void calc_log(char *message, const char *function, int line)
{
        printf("%s at %s::line %d", message, function, line);
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
}

void add_token(struct Tokens *tokens, char *str, enum Type type, enum Bp bp)
{
    struct Token tk;

    switch (type) {
        case PARENTHESIS:
        case OPERATOR:
            tk.val = calc_malloc(sizeof(char) * 2);
            tk.val[0] = str[0];
            tk.val[1] = '\0';
            tk.bp = bp;
            tk.type = type;
            tokens->tokens[tokens->len] = tk;
            tokens->len++;
            break;

        case NUMBER:
            tk.val = calc_malloc((sizeof(char) * strlen(str)) + 1);
            strcpy(tk.val, str);
            tk.type = NUMBER;
            tk.bp = NUM;
            tokens->tokens[tokens->len] = tk;
            tokens->len++;
            break;

        default:
            break;
    }
}

void debug_tokens(struct Tokens *tokens)
{
    const char *lookup_t[] = {"OPERATOR", "NUMBER", "PARENTHESIS", "UNKNOWN"};
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

