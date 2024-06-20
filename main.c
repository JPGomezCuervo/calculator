#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define BUFF_SIZE 1024
#define DELIMITER '$'

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

typedef struct Token
{
    char *val;
    enum Type type;
    enum Bp bp;
} Token;

typedef struct Tokens
{
    Token *tokens;
    size_t len;
} Tokens;

enum Type get_type(char c);

void debug_tokens(Tokens *tokens);

int main(int argsc, char **argsv)
{
    Tokens *tokens = malloc(sizeof(Tokens));

    if (tokens == NULL)
        return 1;

    char *input = malloc(sizeof(char) * BUFF_SIZE);

    if (input == NULL)
    {
        free(tokens);
        return 1;
    }
    else
        input[0] = '\0';

    if (argsc < 2)
    {
        printf("USE: calc <expr>\n");
        free(input);
        free(tokens);
        return 1;
    }

    if (argsc > 2)
    {
        for (int i = 1; i < argsc; i++)
        {
            for (size_t j = 0; j < strlen(argsv[i]); j++) 
            {
                if (isspace(argsv[i][j])) continue;
                strncat(input, &argsv[i][j], 1);
            }
        }
    } 
    else
        strcpy(input, argsv[1]);

    input[strlen(input)] = (char) DELIMITER;
    input[strlen(input)+1] = '\0';

    tokens->tokens = malloc(sizeof(Token) * BUFF_SIZE);
    tokens->len = 0;

    char *temp = malloc(sizeof(char) * 50);
    if (temp == NULL) {
        free(input);
        free(tokens->tokens);
        free(tokens);
        return 1;
    }


    {
        temp[0] = '\0';
        int pos = 0;
        bool prefix = false;

        for (size_t i = 0; i < strlen(input); i++) 
        {
            char c = input[i];
            enum Type t;
            enum Bp bp;
            Token tk;

            switch (c)
            {
                case '(':
                case ')':
                    t = PARENTHESIS;
                    bp = MAX;
                    break;

                case '+':
                case '-':
                    t = OPERATOR;
                    bp = ADD_SUB;
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
                    t = NUMBER;
                    break;
                case '$':
                    {
                        if (pos != 0)
                        {
                            temp[pos] = '\0';
                            tk.val = malloc((sizeof(char) * pos) + 1);
                            strcpy(tk.val, temp);
                            tk.type = NUMBER;
                            tk.bp = NUM;

                            pos = 0;
                            temp[0] = '\0';

                            tokens->tokens[tokens->len] = tk;
                            tokens->len++;
                        }
                        t = LIMIT;
                        break;
                    }
                default:
                    t = UNKNOWN;
                    break;
            }

            if (i == 0 && t == OPERATOR) prefix = true;

            if (t == NUMBER)
            {
                temp[pos] = c;
                pos++;
            }

            if (t == OPERATOR && prefix)
            {
                prefix = false;

                tk.val = malloc(sizeof(char) * 2);
                tk.val[0] = c;
                tk.val[1] = '\0';
                tk.bp = bp;
                tk.type = t;

                tokens->tokens[tokens->len] = tk;
                tokens->len++;
            }

            if (t == OPERATOR && t != LIMIT)
            {
                temp[pos] = '\0';
                tk.val = malloc((sizeof(char) * pos) + 1);
                strcpy(tk.val, temp);
                tk.type = NUMBER;
                tk.bp = NUM;

                pos = 0;
                temp[0] = '\0';

                tokens->tokens[tokens->len] = tk;
                tokens->len++;


                Token optk;
                optk.val = malloc(sizeof(char) * 2);
                optk.val[0] = c;
                optk.val[1] = '\0';
                optk.bp = bp;
                optk.type = t;

                tokens->tokens[tokens->len] = optk;
                tokens->len++;
            }
        }

    }
    debug_tokens(tokens);

    for (size_t i = 0; i < tokens->len; i++) {
        free(tokens->tokens[i].val);
    }
    free(tokens->tokens);
    free(temp);
    free(input);
    free(tokens);
    return 0;
}

void debug_tokens(Tokens *tokens)
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
