#include "calc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

struct Calculator
{
        struct Lexer *tokens;
        struct Leaf *tree;
        char *input;
        size_t input_len;
        enum Calc_err error;
        struct History *history;
        bool history_active;
};

const char *type_names[] = {
        [UNARY_NEG] = "UNARY_NEG",
        [UNARY_POS] = "UNARY_POS",
        [OP_ADD] = "OP_ADD",
        [OP_SUB] = "OP_SUB",
        [OP_MUL] = "OP_MUL",
        [OP_DIV] = "OP_DIV",
        [OPEN_PARENT] = "OPEN_PARENT",
        [CLOSE_PARENT] = "CLOSE_PARENT",
        [LIMIT] = "LIMIT",
        [NUMBER] = "NUMBER",
        [UNKNOWN] = "UNKNOWN"
};

char *calc_err_msg [] =
{
        [ERR_NO_INPUT] = "no input provided",
        [ERR_DIVIDE_BY_ZERO] = "division by zero",
        [ERR_UNKNOWN_OPERATOR] = "unknown operator",
        [ERR_SYNTAX] = "invalid syntax",
};

struct Calculator *init_calculator(size_t history_size)
{
        struct Calculator *calculator = calc_malloc(sizeof(struct Calculator));
        *calculator = (struct Calculator){
                .tokens = NULL,
                .input = NULL,
                .tree = NULL,
                .input_len = 0,
                .error = ERR_NO_ERR,
        };

        if (history_size != 0)
        {
                struct History *p_hist = NULL;
                calculator->history_active = true;

                calculator->history = calc_malloc(sizeof(struct History));
                p_hist = calculator->history;

                p_hist->capacity = history_size;
                p_hist->len = 0;
                p_hist->exprs = calc_malloc(sizeof(struct Expression*) * p_hist->capacity);

                for (size_t i = 0; i < p_hist->capacity; i++)
                {
                        p_hist->exprs[i] = calc_malloc(sizeof(struct Expression));
                        *p_hist->exprs[i] = (struct Expression){
                                .id = 0,
                                .expr = NULL,
                                .result = 0.0,
                        };
                }
        }
        else
                calculator->history_active = false;

        return calculator;
}

double calculate_expr(struct Calculator *h, char *str)
{
        double result = 0;
        int input_index = 0;
        char *psrc = str;

        while (*psrc != '\0')
        {
                if (!isspace(*psrc))
                        h->input_len++;
                psrc++;
        }
        psrc = str;

        h->input = calc_malloc(sizeof(char) * (h->input_len + 2));

        while (*psrc != '\0')
        {
                if (*psrc == DELIMITER)
                        dead(h, ERR_UNKNOWN_OPERATOR);

                if (!isspace(*psrc))
                        h->input[input_index++] = *psrc;
                psrc++;
        }

        h->input[input_index++] = DELIMITER;
        h->input[input_index] = '\0';
        h->input_len = input_index;


        h->tokens = initialize_tokens(h);
        make_tokens(h);
        check_semantics(h);
        h->tree = parse_expr(h, MIN_LIMIT);

        result = eval_tree(h, h->tree);

        if (h->history_active)
        {
                struct History *p_h = h->history;
                p_h->exprs[p_h->len]->expr = calc_malloc(sizeof(char)*(h->input_len + 1));
                *p_h->exprs[p_h->len] = (struct Expression)
                {
                        .expr = strcpy(p_h->exprs[p_h->len]->expr, h->input),
                        .result = result,
                        .id = p_h->len,
                };

                p_h->len++;
        }

        calc_cleanup(h);
        return result;
}

void dead(Calculator *h, enum Calc_err err)
{
        assert(err >= 0);

        fprintf(stderr, "ERROR: ");
        fprintf(stderr, "%s\n", calc_err_msg[err]);
        h->error = err;
        exit(err);
}

void *calc_malloc(size_t len)
{
        void *p = NULL;
        p = malloc(len);

        if (!p) {
                calc_log("Error in allocation", __func__, __LINE__);
                exit(EXIT_FAILURE);
        }

        return p;
}

void *calc_calloc(int num, size_t size)
{
        void *p = NULL;
        p = calloc(num, size);

        if (!p) 
        {
                calc_log("Error in allocation", __func__, __LINE__);
                exit(EXIT_FAILURE);
        }

        return p;
}

void *calc_realloc(void *p, size_t new_size)
{
        p = realloc(p, new_size);

        if (!p)
        {
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

void calc_cleanup(struct Calculator *h)
{

        struct Lexer *tokens = h->tokens;

        if (tokens) 
        {
                /*Reverse loop to take advantage of the Zero Flag cpu optmization*/
                for (int i = (int)tokens->len - 1; i >= 0; i--)
                        free(tokens->chars[i]);

                free(tokens->chars);
                free(tokens);
                tokens = NULL;
        }

        if (h->input) 
        {
                free(h->input);
                h->input = NULL;
        }

        if (h->tree)
        {
                free_tree(h->tree);
                h->tree = NULL;
        }
}

struct Lexer *initialize_tokens(struct Calculator *h)
{
        struct Lexer *tokens = calc_malloc(sizeof(struct Lexer));
        tokens->chars = calc_malloc(sizeof(char*) * h->input_len);
        tokens->curr = 0;

        return tokens;
}

int make_tokens(struct Calculator *h)
{
        int tks_readed = 0;
        struct Lexer *tokens = h->tokens;
        for (size_t i = 0; i < h->input_len; i++)
        {
                enum Type t = get_type(h->input[i]);
                add_token(h, &i, t, tks_readed);
                tks_readed++;
        }
        tokens->chars = calc_realloc(tokens->chars, sizeof(char*) * tks_readed);
        tokens->len = tks_readed;

        return tks_readed;
}

void add_token(struct Calculator *h, size_t *i, enum Type t, size_t tokens_pos) 
{
        struct Lexer *tokens = h->tokens;
        if (t == NUMBER)
        {
                size_t size = 0;
                const char *p_input = &(h->input[*i]);

                while (is_number(*p_input) && *i < h->input_len)
                {
                        size++;
                        p_input++;
                }

                char *str = calc_malloc(sizeof(char) * (size + 1));

                for (size_t j = 0; j < size; j++)
                {
                        str[j] = h->input[*i];
                        (*i)++;
                }

                str[size] = '\0';
                tokens->chars[tokens_pos] = str;
                (*i)--;  // Adjust index since the loop will increment i once more
        }
        else
        {
                tokens->chars[tokens_pos] = calc_malloc(sizeof(char) * 2);
                tokens->chars[tokens_pos][0] = h->input[*i];
                tokens->chars[tokens_pos][1] = '\0';
        }
}

void debug_tokens(struct Lexer *tokens)
{
        for (size_t i = 0; i < tokens->len; i++)
        {
                printf("index: %zu, Value: %s, Type: %s, Precedence: %d\n", 
                                i, 
                                &tokens->chars[i][0],
                                type_names[get_type(tokens->chars[i][0])],
                                get_bp(tokens->chars[i][0])
                      );
        }
}

char *get_next(struct Calculator *h)
{
        struct Lexer *tokens = h->tokens;
        assert(tokens != NULL);

        char *pc = NULL;
        if (*tokens->chars[tokens->curr] != DELIMITER)
        {
                pc = tokens->chars[tokens->curr];
                tokens->curr++;
        }

        return pc;
}

char peek(struct Calculator *h)
{
        struct Lexer *tokens = h->tokens;
        assert(tokens != NULL);

        if (*tokens->chars[tokens->curr] != DELIMITER)
                return *tokens->chars[tokens->curr];

        return DELIMITER;
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

bool is_number(char c)
{
        return isdigit(c) || c == '.';
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
                default:
                        return false;
        }
}

bool is_parenthesis(enum Type t)
{
        return (t == OPEN_PARENT || t == CLOSE_PARENT);
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

struct Leaf *parse_leaf(struct Calculator *h)
{
        char *tk = get_next(h);
        struct Leaf *leaf = NULL;

        if (tk == NULL)
                return leaf;

        enum Type t = get_type(*tk);

        /* checks if is a unary operator */
        if (t == OP_ADD || t == OP_SUB)
        {
                struct Leaf *right = parse_leaf(h); 
                leaf = make_leaf(tk);
                leaf->right = right;
        }
        else if (t == OPEN_PARENT)
        {
                leaf = parse_expr(h, MIN_LIMIT);
                /* consumes close parenthesis */
                get_next(h); 
        }
        else 
                leaf = make_leaf(tk);

        return leaf;
}

struct Leaf *parse_expr(struct Calculator *h, enum Bp bp)
{
        struct Leaf *left = parse_leaf(h);
        struct Leaf *node = NULL;

        while (true)
        {
                node = increasing_prec(h, left, bp);

                if (left == node)
                        break;
                left = node;
        }
        return left;
}

struct Leaf *increasing_prec(struct Calculator *h,struct Leaf *left, enum Bp min_bp)
{
        char next = peek(h);
        enum Type t = get_type(next);
        enum Bp bp = get_bp(next);


        if (next == DELIMITER || t == CLOSE_PARENT)
                return left;

        if (is_operator(t))
        {
                while (bp >= min_bp) 
                {
                        char *op = get_next(h);
                        struct Leaf *right = parse_expr(h, bp);
                        left = make_binary_expr(op,left, right);
                        next = peek(h);
                        t = get_type(next);

                        if (next == DELIMITER || t == CLOSE_PARENT)
                                break;
                }
        }
        return left;
}

void check_semantics(struct Calculator *h)
{
        assert(h != NULL);
        assert(h->tokens->chars != NULL);
        struct Lexer *tokens = h->tokens;

        bool was_operator = false;

        for (size_t i = 0; i < h->tokens->len; i++)
        {
                enum Type curr_t = get_type(*tokens->chars[i]);

                if (i == 0 && (curr_t == OP_MUL || curr_t == OP_DIV))
                        dead(h, ERR_SYNTAX);

                if (is_number(*tokens->chars[i]))
                {
                        was_operator = false;
                        continue;
                }

                if (was_operator)
                {
                        if (is_operator(curr_t)) 
                                dead(h, ERR_SYNTAX);

                        if (curr_t == LIMIT)
                                dead(h, ERR_SYNTAX);

                        if (curr_t == CLOSE_PARENT)
                                dead(h, ERR_SYNTAX);
                }

                was_operator = is_operator(curr_t);
        }
}

double eval_tree(Calculator *h, struct Leaf *tree)
{
        assert(tree != NULL);
        assert(tree->value != NULL);

        double lhs = 0.0, rhs = 0.0;
        enum Type t = get_type(*tree->value);

        if (t == NUMBER) 
                return strtod(tree->value, NULL);

        lhs = tree->left != NULL ? eval_tree(h, tree->left) : 0;
        rhs = tree->right != NULL ? eval_tree(h, tree->right) : 0;

        switch (t) 
        {
                case OP_ADD:
                        return lhs + rhs;
                case OP_SUB:
                        return lhs - rhs;
                case OP_MUL:
                        return lhs * rhs;
                case OP_DIV:
                        if (rhs == 0)
                                dead(h, ERR_DIVIDE_BY_ZERO);

                        return lhs / rhs;
                case UNARY_NEG:
                        return -rhs;
                default:
                        dead(h, ERR_UNKNOWN_OPERATOR);
                        return 0.0;
        }
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

enum Calc_err error_code(Calculator *h)
{
        return h->error;
}

char *error_message(Calculator *h)
{
        if (h->error != ERR_NO_ERR)
                return calc_err_msg[h->error];
        else
                return NULL;
}

void destroy_calculator(Calculator *h)
{

        if (h->history_active)
        {
                for (size_t i = 0; i < h->history->capacity; i++) 
                {
                        free(h->history->exprs[i]->expr);
                        free(h->history->exprs[i]);
                }
                free(h->history->exprs);
                free(h->history);
        }

        free(h);
        h = NULL;
}

struct Expression **get_history(struct Calculator *h)
{
        if (h->history)
                return h->history->exprs;

        return NULL;
}

size_t get_history_len(struct Calculator *h)
{
        if (h->history)
                return h->history->len;

        return 0;
}
