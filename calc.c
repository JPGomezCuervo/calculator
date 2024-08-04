#include "calc_internal.h"
#include "calc.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#define HEAP_SIZE 4096
#define ALIGNMENT 64

struct Calculator
{
        struct Lexer *tokens;
        struct Leaf *tree;
        char *input;
        size_t input_len;
        error_code error;
        struct History *history;
        bool history_active;
        struct Leaf pool;
        struct Calculator_heap *heap;
};

const char *type_names[] = {
        [TokenType_UNKNOWN] = "TNUMBER",
        [TokenType_UNARY_NEG] = "UNARY_NEG",
        [TokenType_UNARY_POS] = "UNARY_POS",
        [TokenType_OP_ADD] = "OP_ADD",
        [TokenType_OP_SUB] = "OP_SUB",
        [TokenType_OP_MUL] = "OP_MUL",
        [TokenType_OP_DIV] = "OP_DIV",
        [TokenType_OPEN_PARENT] = "OPEN_PARENT",
        [TokenType_CLOSE_PARENT] = "CLOSE_PARENT",
        [TokenType_LIMIT] = "LIMIT",
        [TokenType_NUMBER] = "NUMBER",
};

char *calc_err_msg [] =
{
        [ERR_NO_INPUT] = "no input provided",
        [ERR_DIVIDE_BY_ZERO] = "division by zero",
        [ERR_UNKNOWN_OPERATOR] = "unknown operator",
        [ERR_SYNTAX] = "invalid syntax",
        [ERR_HEAP_ALLOC] = "failed heap alloc",
};

struct Calculator *init_calculator(size_t history_size)
{
        struct Calculator *calculator = calc_malloc(sizeof(struct Calculator));
        uint8_t *p_pool = NULL;
        struct Calculator_heap *p_heap = NULL;
        struct Leaf *p_leaf = NULL;
        int rc = 0;

        *calculator = (struct Calculator){
                .tokens = NULL,
                        .input = NULL,
                        .tree = NULL,
                        .input_len = 0,
                        .error = ERR_NO_ERR,
                        .heap = NULL,
                        .pool =
                        {
                                .data = {.is_number = true, .val.number = 0.0},
                                .left = &calculator->pool,
                                .right = &calculator->pool,
                        }
        };

        rc = posix_memalign((void**)&(calculator->heap), ALIGNMENT, HEAP_SIZE);

        if (rc != 0)
                exit(ERR_HEAP_ALLOC);

        memset(calculator->heap, 0, HEAP_SIZE);

        p_heap = calculator->heap;
        p_heap->next_heap = NULL;

        p_pool = (uint8_t*) p_heap + 32;

        for (; p_pool < ((uint8_t*) p_heap) + HEAP_SIZE; p_pool += 32)
        {
                p_leaf = (struct Leaf *) p_pool;
                p_leaf->right = calculator->pool.right;
                p_leaf->left = calculator->pool.left;
                calculator->pool.left->right = p_leaf;
                calculator->pool.right->left = p_leaf;
        }

        if (history_size != 0)
        {
                struct History *p_hist = NULL;
                calculator->history_active = true;

                calculator->history = calc_malloc(sizeof(struct History));
                p_hist = calculator->history;

                p_hist->capacity = history_size;
                p_hist->len = 0;
                p_hist->exprs = calc_malloc(sizeof(struct Expression*) * p_hist->capacity);

                for (int i = (int) p_hist->capacity - 1; i >= 0; i--)
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
        if (str == NULL)
        {
                error_code err = h->error;
                dead(h, ERR_NO_INPUT);
                destroy_calculator(h);
                exit(err);
        }

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
        h->tree = parse_expr(h, BP_MIN_LIMIT);

        result = eval_tree(h, h->tree);

        if (h->history_active)
        {
                struct History *p_hist = h->history;
                if (!(p_hist->len < p_hist->capacity))
                {
                        p_hist->len = 0;
                        free(p_hist->exprs[p_hist->len]->expr);
                }

                p_hist->exprs[p_hist->len]->expr = calc_malloc(sizeof(char)*(h->input_len));

                size_t len = strlen(h->input);
                *p_hist->exprs[p_hist->len] = (struct Expression)
                {
                        .expr = strncpy(p_hist->exprs[p_hist->len]->expr, h->input, len),
                                .result = result,
                                .id = p_hist->len,
                };

                p_hist->exprs[p_hist->len]->expr[len - 1] = '\0';
                p_hist->len++;
        }

        calc_cleanup(h);
        return result;
}

struct Leaf *get_free_leaf(Calculator *h)
{
        int rc = 0;
        struct Leaf *p_leaf = NULL;
        struct Calculator_heap *p_heap = NULL;
        uint8_t *p = NULL;
        
        if (h->pool.left == h->pool.right)
        {
                rc = posix_memalign((void**)&p, ALIGNMENT, HEAP_SIZE);

                if (rc != 0)
                        exit(ERR_HEAP_ALLOC);
                p_heap = h->heap;

                while (p_heap->next_heap != NULL)
                {
                        p_heap = (struct Calculator_heap *)p_heap->next_heap;
                }

                p_heap->next_heap = p;

                p = p + 32;
                
                for (; p < ((uint8_t *) p_heap->next_heap) + HEAP_SIZE ; p += 32) 
                {
                        p_leaf = (struct Leaf *) p;
                        p_leaf->right = h->pool.right;
                        p_leaf->left = h->pool.left;
                        h->pool.left->right = p_leaf;
                        h->pool.right->left = p_leaf;
                }
        }

        p_leaf = h->pool.right;
        h->pool.right = p_leaf->right;
        p_leaf->right = p_leaf->left;

        return p_leaf;
}

void recycle_leaf(struct Calculator *h, struct Leaf *p_leaf)
{
        p_leaf->left = p_leaf;
        p_leaf->right = h->pool.right;
        h->pool.right = p_leaf;
}

void dead(Calculator *h, error_code err)
{
        fprintf(stderr, "ERROR: ");
        fprintf(stderr, "%s\n", calc_err_msg[err]);
        h->error = err;
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

void free_tree(struct Calculator *h, struct Leaf *tree)
{
        if (!tree)
                return;
        free_tree(h, tree->right);
        free_tree(h, tree->left);

        recycle_leaf(h, tree);
}

void calc_cleanup(struct Calculator *h)
{

        struct Lexer *tokens = h->tokens;

        if (tokens) 
        {
                /*Reverse loop to take advantage of the Zero Flag cpu optmization*/
                for (int i = (int)tokens->len - 1; i >= 0; i--)
                        free(tokens->data[i]);

                free(tokens->data);
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
                free_tree(h, h->tree);
                h->tree = NULL;
        }
}

struct Lexer *initialize_tokens(struct Calculator *h)
{
        struct Lexer *tokens = calc_malloc(sizeof(struct Lexer));
        tokens->data = calc_malloc(sizeof(struct Data*) * h->input_len);
        tokens->curr = 0;

        return tokens;
}

int make_tokens(struct Calculator *h)
{
        int tks_readed = 0;
        struct Lexer *tokens = h->tokens;
        for (size_t i = 0; i < h->input_len; i++)
        {
                token_type t = get_type(
                                (struct Data){
                                .is_number = false,
                                .val = {.sign = {h->input[i], '\0'}}
                                }
                                );

                add_token(h, &i, t);
                tks_readed++;
        }
        tokens->data = calc_realloc(tokens->data, sizeof(struct Data*) * tks_readed);
        tokens->len = tks_readed;
        tokens->curr = 0;

        return tks_readed;
}

void add_token(struct Calculator *h, size_t *i, token_type t) 
{
        struct Lexer *tokens = h->tokens;
        struct Data **data_array = tokens->data;
        struct Data *current_data = NULL;

        if (t == TokenType_NUMBER)
        {
                const char *p_input = &(h->input[*i]);
                double num = 0;
                size_t iterations = 0;

                while (is_number(*p_input) && *i < h->input_len)
                {
                        iterations++;
                        p_input++;
                }

                num = strtod(&(h->input[*i]), NULL);

                current_data = calc_malloc(sizeof(struct Data));
                current_data->is_number = true;
                current_data->val.number = num;

                data_array[tokens->curr] = current_data;
                tokens->curr++;
                *i += iterations - 1; // Adjust index since the loop will increment i once more
        }
        else
        {
                current_data = calc_malloc(sizeof(struct Data));
                current_data->is_number = false;
                current_data->val.sign[0] = h->input[(*i)];
                current_data->val.sign[1] = '\0';

                data_array[tokens->curr] = current_data;
                tokens->curr++;
        }
}

void debug_tokens(struct Calculator *h)
{
        struct Lexer *tokens = h->tokens;
        for (size_t i = 0; i < tokens->len; i++)
        {
                token_type t = get_type(*tokens->data[i]);
                if (t == TokenType_NUMBER)
                {
                        printf("index: %zu, Value: %2.f, Type: %s, Precedence: %d\n", 
                                        i, 
                                        tokens->data[i]->val.number,
                                        type_names[t],
                                        get_bp(*tokens->data[i])
                              );
                }
                else 
                {
                        printf("index: %zu, Value: %s, Type: %s, Precedence: %d\n", 
                                        i, 
                                        tokens->data[i]->val.sign,
                                        type_names[t],
                                        get_bp(*tokens->data[i])
                              );
                }
        }
}

struct Data *get_next(struct Calculator *h)
{
        struct Lexer *tokens = h->tokens;
        assert(tokens != NULL);
        token_type curr_t = get_type(*tokens->data[tokens->curr]); 

        struct Data *pc = NULL;
        if (curr_t != TokenType_LIMIT)
        {
                pc = tokens->data[tokens->curr];
                tokens->curr++;
        }

        return pc;
}

struct Data peek(struct Calculator *h)
{
        struct Lexer *tokens = h->tokens;
        assert(tokens != NULL);

        return *tokens->data[tokens->curr];
}

token_type get_type(struct Data c) 
{

        if (c.is_number) 
                return TokenType_NUMBER;

        switch (c.val.sign[0]) 
        {
                case '+':
                        return TokenType_OP_ADD;
                case '-':
                        return TokenType_OP_SUB;
                case '*':
                        return TokenType_OP_MUL;
                case '/':
                        return TokenType_OP_DIV;
                case '(':
                        return TokenType_OPEN_PARENT;
                case ')':
                        return TokenType_CLOSE_PARENT;
                case '?':
                        return TokenType_LIMIT;
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                        return TokenType_NUMBER;
                default:
                        return TokenType_UNKNOWN;
        }
}

binary_power get_bp(struct Data c) 
{

        if (c.is_number) 
                return BP_NUMBER;

        switch (c.val.sign[0]) 
        {
                case ')':
                case '(':
                        return BP_MIN_LIMIT;
                case '+':
                case '-':
                        return BP_ADD_SUB;
                case '*':
                case '/':
                        return BP_MUL_DIV;
                default:
                        return BP_UNKNOWN;
        }
}

inline bool is_number(char c)
{
        return isdigit(c) || c == '.';
}

bool is_operator(token_type t)
{
        switch (t)
        {
                case TokenType_OP_ADD:
                case TokenType_OP_SUB:
                case TokenType_OP_MUL:
                case TokenType_OP_DIV:
                        return true;
                default:
                        return false;
        }
}

inline bool is_parenthesis(token_type t)
{
        return (t == TokenType_OPEN_PARENT || t == TokenType_CLOSE_PARENT);
}

struct Leaf *make_leaf(struct Calculator *h, struct Data *tk)
{
        struct Leaf *leaf = get_free_leaf(h);
        leaf->data = *tk;
        leaf->left = NULL;
        leaf->right = NULL;
        return leaf;
}

struct Leaf *make_binary_expr(struct Calculator *h, struct Data *op, struct Leaf *left, struct Leaf *right)
{
        struct Leaf *leaf = get_free_leaf(h);
        leaf->data = *op;
        leaf->left = left;
        leaf->right = right;

        return leaf;
}

struct Leaf *parse_leaf(struct Calculator *h)
{
        struct Data *tk = get_next(h);
        struct Leaf *leaf = NULL;

        if (tk == NULL)
                return leaf;

        token_type t = get_type(*tk);

        /* checks if is a unary operator */
        if (t == TokenType_OP_ADD || t == TokenType_OP_SUB)
        {
                struct Leaf *right = parse_leaf(h); 
                leaf = make_leaf(h, tk);
                leaf->right = right;
        }
        else if (t == TokenType_OPEN_PARENT)
        {
                leaf = parse_expr(h, BP_MIN_LIMIT);
                /* consumes close parenthesis */
                get_next(h); 
        }
        else 
                leaf = make_leaf(h, tk);

        return leaf;
}

struct Leaf *parse_expr(struct Calculator *h, binary_power b)
{
        struct Leaf *left = parse_leaf(h);
        struct Leaf *node = NULL;

        while (true)
        {
                node = increasing_prec(h, left, b);

                if (left == node)
                        break;
                left = node;
        }
        return left;
}

struct Leaf *increasing_prec(struct Calculator *h,struct Leaf *left, binary_power min_bp)
{
        struct Data next = peek(h);
        token_type t = get_type(next);
        binary_power bp = get_bp(next);


        if (t == TokenType_LIMIT|| t == TokenType_CLOSE_PARENT)
                return left;

        if  (t == TokenType_OPEN_PARENT && t == TokenType_NUMBER)
        {
                struct Data new_data =
                {
                        .is_number = false,
                        .val.sign = "*",
                };

                h->tokens->data[h->tokens->curr] = &new_data; 
                next = peek(h);
                t = get_type(next);
                bp = get_bp(next);
        }

        if (is_operator(t))
        {
                while (bp >= min_bp) 
                {
                        struct Data *op = get_next(h);
                        struct Leaf *right = parse_expr(h, bp);
                        left = make_binary_expr(h, op,left, right);
                        next = peek(h);
                        t = get_type(next);

                        if (t == TokenType_LIMIT || t == TokenType_CLOSE_PARENT)
                                break;
                }
        }
        return left;
}

void check_semantics(struct Calculator *h)
{
        assert(h != NULL);
        struct Lexer *tokens = h->tokens;

        bool was_operator = false;

        for (size_t i = 0; i < h->tokens->len; i++)
        {
                token_type curr_t; 
                if (tokens->data[i]->is_number)
                        curr_t = TokenType_NUMBER;
                else
                        curr_t = get_type(*tokens->data[i]);

                if (i == 0 && (curr_t == TokenType_OP_MUL || curr_t == TokenType_OP_DIV))
                        dead(h, ERR_SYNTAX);

                if (curr_t == TokenType_NUMBER)
                {
                        was_operator = false;
                        continue;
                }

                if (was_operator)
                {
                        if (is_operator(curr_t)) 
                                dead(h, ERR_SYNTAX);

                        if (curr_t == TokenType_LIMIT)
                                dead(h, ERR_SYNTAX);

                        if (curr_t == TokenType_CLOSE_PARENT)
                                dead(h, ERR_SYNTAX);
                }

                was_operator = is_operator(curr_t);
        }
}

double eval_tree(Calculator *h, struct Leaf *tree)
{
        assert(tree != NULL);

        double lhs = 0.0, rhs = 0.0;
        token_type t = get_type(tree->data);

        if (t == TokenType_NUMBER) 
                return tree->data.val.number;

        lhs = tree->left != NULL ? eval_tree(h, tree->left) : 0;
        rhs = tree->right != NULL ? eval_tree(h, tree->right) : 0;

        switch (t) 
        {
                case TokenType_OP_ADD:
                        return lhs + rhs;
                case TokenType_OP_SUB:
                        return lhs - rhs;
                case TokenType_OP_MUL:
                        return lhs * rhs;
                case TokenType_OP_DIV:
                        if (rhs == 0)
                                dead(h, ERR_DIVIDE_BY_ZERO);

                        return lhs / rhs;
                case TokenType_UNARY_NEG:
                        return -rhs;
                default:
                        dead(h, ERR_UNKNOWN_OPERATOR);
                        return 0.0;
        }
}

void debug_tree(struct Calculator *h)
{
        assert(h->tokens != NULL);
        print_tree(h->tree, "");
}

void print_tree(struct Leaf *leaf, const char *indent)
{
        if (leaf == NULL)
                return;

        if  (leaf->data.is_number)
                printf("%sHead: %2.f\n", indent, leaf->data.val.number);
        else
                printf("%sHead: %s\n", indent, leaf->data.val.sign);

        if (leaf->left)
        {
                printf("%sLeft:\n", indent);
                char new_indent[256];
                snprintf(new_indent, sizeof(new_indent), "%s    ", indent);
                print_tree(leaf->left, new_indent);
        }

        if (leaf->right)
        {
                printf("%sRight:\n", indent);
                char new_indent[256];
                snprintf(new_indent, sizeof(new_indent), "%s    ", indent);
                print_tree(leaf->right, new_indent);
        }
}

error_code get_error_code(Calculator *h)
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

        uint8_t *p, *p_next;
        p = (uint8_t*) h->heap;

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

        while (p != NULL)
        {
                p_next = (uint8_t *)((struct Calculator_heap *)p)->next_heap;
                free(p);
                p = p_next;
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
