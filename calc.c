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
        calculator->tokens = NULL;
        calculator->input = NULL;
        calculator->tree = NULL;
        calculator->input_len = 0;
        calculator->error = ERR_NO_ERR;

        if (history_size != 0)
        {
                calculator->history_active = true;
                calculator->history = calc_malloc(sizeof(struct History));
                calculator->history->capacity = history_size;
                calculator->history->len = 0;

                calculator->history->exprs = calc_malloc(sizeof(struct Expression*) * calculator->history->capacity);
                for (size_t i = 0; i < calculator->history->capacity; i++)
                {
                        calculator->history->exprs[i] = calc_malloc(sizeof(struct Expression));
                        calculator->history->exprs[i]->id = 0;
                        calculator->history->exprs[i]->expr = NULL;
                        calculator->history->exprs[i]->id = 0;
                }
        }
        else
                calculator->history_active = false;

        return calculator;
}

double calculate_expr(struct Calculator *handler, char *str)
{
        double result = 0;
        int input_index = 0;
        char *psrc = str;

        while (*psrc != '\0')
        {
                if (!isspace(*psrc))
                        handler->input_len++;
                psrc++;
        }
        psrc = str;

        handler->input = calc_malloc(sizeof(char) * (handler->input_len + 2));

        while (*psrc != '\0')
        {
                if (*psrc == DELIMITER)
                        dead(handler, ERR_UNKNOWN_OPERATOR);

                if (!isspace(*psrc))
                        handler->input[input_index++] = *psrc;
                psrc++;
        }

        handler->input[input_index++] = DELIMITER;
        handler->input[input_index] = '\0';
        handler->input_len = input_index;


        handler->tokens = initialize_tokens(handler);
        make_tokens(handler);
        check_semantics(handler);
        handler->tree = parse_expr(handler, MIN_LIMIT);

        result = eval_tree(handler, handler->tree);

        if (handler->history_active)
        {
                struct History *p_h = handler->history;
                p_h->exprs[p_h->len]->expr = calc_malloc(sizeof(char)*(handler->input_len + 1));
                p_h->exprs[p_h->len]->expr = strcpy(p_h->exprs[p_h->len]->expr, handler->input);
                p_h->exprs[p_h->len]->result = result;
                p_h->exprs[p_h->len]->id = p_h->len;
                p_h->len++;
        }

        calc_cleanup(handler);
        return result;
}

void dead(Calculator *handler, enum Calc_err err)
{
        assert(err >= 0);

        fprintf(stderr, "ERROR: ");
        fprintf(stderr, "%s\n", calc_err_msg[err]);
        handler->error = err;
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

void calc_cleanup(struct Calculator *handler)
{

        struct Lexer *tokens = handler->tokens;

        if (tokens) 
        {
                /*Reverse loop to take advantage of the Zero Flag cpu optmization*/
                for (int i = (int)tokens->len - 1; i >= 0; i--)
                        free(tokens->chars[i]);

                free(tokens->chars);
                free(tokens);
                tokens = NULL;
        }

        if (handler->input) 
        {
                free(handler->input);
                handler->input = NULL;
        }

        if (handler->tree)
        {
                free_tree(handler->tree);
                handler->tree = NULL;
        }
}

struct Lexer *initialize_tokens(struct Calculator *handler)
{
        struct Lexer *tokens = calc_malloc(sizeof(struct Lexer));
        tokens->chars = calc_malloc(sizeof(char*) * handler->input_len);
        tokens->curr = 0;

        return tokens;
}

int make_tokens(struct Calculator *handler)
{
        int tks_readed = 0;
        struct Lexer *tokens = handler->tokens;
        for (size_t i = 0; i < handler->input_len; i++)
        {
                enum Type t = get_type(handler->input[i]);
                add_token(handler, &i, t, tks_readed);
                tks_readed++;
        }
        tokens->chars = calc_realloc(tokens->chars, sizeof(char*) * tks_readed);
        tokens->len = tks_readed;

        return tks_readed;
}

void add_token(struct Calculator *handler, size_t *i, enum Type t, size_t tokens_pos) 
{
        struct Lexer *tokens = handler->tokens;
        if (t == NUMBER)
        {
                size_t size = 0;
                const char *p_input = &(handler->input[*i]);

                while (is_number(*p_input) && *i < handler->input_len)
                {
                        size++;
                        p_input++;
                }

                char *str = calc_malloc(sizeof(char) * (size + 1));

                for (size_t j = 0; j < size; j++)
                {
                        str[j] = handler->input[*i];
                        (*i)++;
                }

                str[size] = '\0';
                tokens->chars[tokens_pos] = str;
                (*i)--;  // Adjust index since the loop will increment i once more
        }
        else
        {
                tokens->chars[tokens_pos] = calc_malloc(sizeof(char) * 2);
                tokens->chars[tokens_pos][0] = handler->input[*i];
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

char *get_next(struct Calculator *handler)
{
        struct Lexer *tokens = handler->tokens;
        assert(tokens != NULL);

        char *pc = NULL;
        if (*tokens->chars[tokens->curr] != DELIMITER)
        {
                pc = tokens->chars[tokens->curr];
                tokens->curr++;
        }

        return pc;
}

char peek(struct Calculator *handler)
{
        struct Lexer *tokens = handler->tokens;
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

struct Leaf *parse_leaf(struct Calculator *handler)
{
        char *tk = get_next(handler);
        struct Leaf *leaf = NULL;

        if (tk == NULL)
                return leaf;

        enum Type t = get_type(*tk);

        if (t == OP_ADD || t == OP_SUB)
        {
                struct Leaf *right = parse_leaf(handler); 
                leaf = make_leaf(tk);
                leaf->right = right;
        }
        else if (t == OPEN_PARENT)
        {
                leaf = parse_expr(handler, MIN_LIMIT);
                /* consumes close parenthesis */
                get_next(handler); 
        }
        else 
        {
                leaf = make_leaf(tk);
        }

        return leaf;
}

struct Leaf *parse_expr(struct Calculator *handler, enum Bp bp)
{
        struct Leaf *left = parse_leaf(handler);
        struct Leaf *node = NULL;

        while (true)
        {
                node = increasing_prec(handler, left, bp);

                if (left == node)
                        break;
                left = node;
        }
        return left;
}

struct Leaf *increasing_prec(struct Calculator *handler,struct Leaf *left, enum Bp min_bp)
{
        char next = peek(handler);
        enum Type t = get_type(next);
        enum Bp bp = get_bp(next);


        if (next == DELIMITER || t == CLOSE_PARENT)
                return left;

        if (is_operator(t))
        {
                while (bp >= min_bp) 
                {
                        char *op = get_next(handler);
                        struct Leaf *right = parse_expr(handler, bp);
                        left = make_binary_expr(op,left, right);
                        next = peek(handler);
                        t = get_type(next);

                        if (next == DELIMITER || t == CLOSE_PARENT)
                                break;
                }
        }
        return left;
}

void check_semantics(struct Calculator *handler)
{
        struct Lexer *tokens = handler->tokens;
        assert(tokens->chars != NULL);

        bool was_operator = false;

        for (size_t i = 0; i < tokens->len; i++)
        {
                enum Type curr_t = get_type(*tokens->chars[i]);

                if (i == 0 && (curr_t == OP_MUL || curr_t == OP_DIV))
                        dead(handler, ERR_SYNTAX);

                if (is_number(*tokens->chars[i]))
                {
                        was_operator = false;
                        continue;
                }

                if (was_operator)
                {
                        if (is_operator(curr_t)) 
                                dead(handler, ERR_SYNTAX);

                        if (curr_t == LIMIT)
                                dead(handler, ERR_SYNTAX);

                        if (curr_t == CLOSE_PARENT)
                                dead(handler, ERR_SYNTAX);
                }

                was_operator = is_operator(curr_t);
        }
}

double eval_tree(Calculator *handler, struct Leaf *tree)
{
        assert(tree != NULL);
        assert(tree->value != NULL);

        double lhs = 0.0, rhs = 0.0;
        enum Type t = get_type(*tree->value);

        if (t == NUMBER) 
                return strtod(tree->value, NULL);

        lhs = tree->left != NULL ? eval_tree(handler, tree->left) : 0;
        rhs = tree->right != NULL ? eval_tree(handler, tree->right) : 0;

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
                                dead(handler, ERR_DIVIDE_BY_ZERO);

                        return lhs / rhs;
                case UNARY_NEG:
                        return -rhs;
                default:
                        dead(handler, ERR_UNKNOWN_OPERATOR);
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

enum Calc_err error_code(Calculator *handler)
{
        return handler->error;
}

char *error_message(Calculator *handler)
{
        if (handler->error != ERR_NO_ERR)
                return calc_err_msg[handler->error];
        else
                return NULL;
}

void destroy_calculator(Calculator *handler)
{

        if (handler->history_active)
        {
                for (size_t i = 0; i < handler->history->capacity; i++) 
                {
                        free(handler->history->exprs[i]->expr);
                        free(handler->history->exprs[i]);
                }
                free(handler->history->exprs);
                free(handler->history);
        }

        free(handler);
        handler = NULL;
}

struct Expression **get_history(struct Calculator *handler)
{
        if (handler->history)
                return handler->history->exprs;

        return NULL;
}

size_t get_history_len(struct Calculator *handler)
{
        if (handler->history)
                return handler->history->len;

        return 0;
}
