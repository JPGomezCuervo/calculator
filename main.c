#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define BUFF_SIZE 102
#define DELIMITER '?'
#define TEMP_STR 51

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

struct Token
{
        char *val;
        enum Type type;
        enum Bp bp;
};

struct Tokens
{
        struct Token *tokens;
        size_t len;
};


// functions

void    debug_tokens(struct Tokens *tokens);
void    *calc_malloc(size_t len);
void    calc_log(char *message, const char *function, int line);
void    calc_cleanup();
void    add_token(struct Tokens *tokens, char *str, enum Type type, enum Bp bp);

struct Tokens *tokens;
char *input;
size_t input_len;

int main(int argsc, char **argsv)
{
        atexit(calc_cleanup);


        tokens = calc_malloc(sizeof(struct Tokens));

        input = calc_malloc(sizeof(char) * BUFF_SIZE);

        input[BUFF_SIZE] = '\0';


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

        return 0;
}

void debug_tokens(struct Tokens *tokens)
{
        const char *lookup_t[] = {
                "OPERATOR",
                "NUMBER",
                "PARENTHESIS",
                "UNKNOWN"
        };

        for (size_t i = 0; i < tokens->len; i++) 
        {
                printf("Index: %zu, Value: %s, Type: %s, Precedence: %d\n", 
                                i, 
                                tokens->tokens[i].val,
                                lookup_t[tokens->tokens[i].type],
                                tokens->tokens[i].bp
                      );
        }
}

void *calc_malloc(size_t len)
{
        void *p;

        if (!(p = malloc(len))) 
        {
                calc_log("Error in allocation", __func__, __LINE__);
                exit(EXIT_FAILURE);
        }

        return p;
}

void add_token(struct Tokens *tokens, char *str, enum Type type, enum Bp bp)
{
        struct Token tk;

        switch (type)
        {
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
