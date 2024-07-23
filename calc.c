#include "calc_internal.h"
#include "calc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#define HEAP_SIZE 4096
struct CalculatorHeap
{
        unsigned char *next_heap;
};

struct Calculator
{
        struct Lexer *tokens;
        struct Leaf *tree;
        char *input;
        size_t input_len;
        enum Calc_err error;
        struct History *history;
        bool history_active;
        struct Leaf pool;
        struct CalculatorHeap* heap;
};

const char *type_names[] = {
        [TokenType_UNKNOWN] = "UNKNOWN",
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
};

struct Calculator *init_calculator(size_t history_size)
{
        int rc;
        struct CalculatorHeap *p_heap;
        struct Calculator *calculator = NULL;
        unsigned char *p;
        struct Leaf *p_leaf; 
        calculator = calc_malloc(sizeof(struct Calculator));

        if (calculator)
        {
                *calculator = (struct Calculator){
                .tokens = NULL,
                .input = NULL,
                .tree = NULL,
                .input_len = 0,
                .error = ERR_NO_ERR,
                .history = NULL,
                .pool = (struct Leaf){
                                .left = NULL,
                                .right = NULL,
                                .data.number = 0.0
                        },
                .heap = NULL
                };
        }

        // initialize pool
        // 1. generate a space with HEAP_SIZE
        // 2. initialize the heap
        // 3. initialize leaf and push into pool
        calculator->pool.left = &(calculator->pool);
        calculator->pool.right = &(calculator->pool);
        
        // 1. generate a space with HEAP_SIZE
        rc = posix_memalign((void**)&calculator->heap, 64, HEAP_SIZE); 
        if (rc != 0)
        {
                exit(EXIT_FAILURE);
        }
        memset(calculator->heap, 0, HEAP_SIZE);
        
        // 2. initialize the heap
        //    We use first 8 bytes to store the pointer to next free leaf. At this moment we have only one leaf.
        p_heap = calculator->heap;
        p_heap->next_heap = NULL;

        // 3. initialize leaf and push into pool
        //    We start from the second 32 bytes to store the leaf data. In this case, each 64 bytes cache line can store 2 leafs, except the first leaf.
        for (p = ((unsigned char *)calculator->heap) + 32; p < ((unsigned char *) calculator->heap) + HEAP_SIZE; p += 32)
        {
                p_leaf = (struct Leaf *)p;
                // insert into pool as link list
                p_leaf->left = calculator->pool.left;
                p_leaf->right = calculator->pool.right;
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
        {
                if (calculator)
                {
                        free(calculator);
                        calculator = NULL;
                }
        }

        return calculator;
}

double calculate_expr(struct Calculator *h, char *str)
{
        double result = 0;
        int input_index = 0;
        char *psrc = str;
        
        if (str == NULL)
        {
                enum Calc_err err = h->error;
                dead(h, ERR_NO_INPUT);
                destroy_calculator(h);
                exit(err);
        }

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

void dead(Calculator *h, enum Calc_err err)
{
        assert(err >= 0);

        fprintf(stderr, "ERROR: ");
        fprintf(stderr, "%s\n", calc_err_msg[err]);
        h->error = err;
        // exit(err);
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
                unsigned char t = get_type(h->input[i]);
                add_token(h, &i, t, tks_readed);
                tks_readed++;
        }
        tokens->chars = calc_realloc(tokens->chars, sizeof(char*) * tks_readed);
        tokens->len = tks_readed;

        return tks_readed;
}

void add_token(struct Calculator *h, size_t *i, unsigned char  t, size_t tokens_pos) 
{
        struct Lexer *tokens = h->tokens;
        if (t == TokenType_NUMBER)
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

void debug_tokens(struct Calculator *h)
{
        struct Lexer *tokens = h->tokens;
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

unsigned char get_type(char c) 
{

        switch (c) 
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

unsigned char get_bp(char c) 
{

        switch (c) 
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

bool is_operator(unsigned char t)
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

bool is_parenthesis(unsigned char t)
{
        return (t == TokenType_OPEN_PARENT || t == TokenType_CLOSE_PARENT);
}

struct Leaf *get_free_leaf(struct Calculator *h, char *tk)
{
        int rc;
        struct Leaf *p_leaf = NULL;
        unsigned char *p;
        struct CalculatorHeap *p_heap;
        if (h == NULL)
        {
                p_leaf = NULL;
        };

        // no free leafs, we need to generate a new one heap for new leave
        if (h->pool.left == h->pool.right)
        {
                rc = posix_memalign((void**)&p, 64, HEAP_SIZE);
                if (rc != 0)
                {
                        exit(EXIT_FAILURE);
                }
                memset(p, 0, HEAP_SIZE);
                
                // Link the new heap to the previous heap
                p_heap = h->heap;
                while(p_heap->next_heap != NULL)
                {
                        p_heap = (struct CalculatorHeap *) p_heap->next_heap;
                }
                p_heap->next_heap = p;

                // initialize the leaf and push into pool
                p_heap = (struct CalculatorHeap *) p;
                for (p = ((unsigned char *) p_heap) + 32; p < ((unsigned char *) p_heap) + HEAP_SIZE; p += 32)
                {
                        p_leaf = (struct Leaf *)p;
                        // insert into pool as link list
                        p_leaf->left = h->pool.left;
                        p_leaf->right = h->pool.right;
                        h->pool.left->right = p_leaf;
                        h->pool.right->left = p_leaf;
                }
        }

        // Get a new leaf from pool
        p_leaf = h->pool.right;
        p_leaf->left->right = p_leaf->right;
        p_leaf->right->left = p_leaf->left;
        p_leaf->left = NULL;
        p_leaf->right = NULL;
        p_leaf->value = tk;

EXIT:
        return p_leaf; 
}

void recycle_leaf(struct Calculator *h, struct Leaf *p_leaf)
{
        p_leaf->left = h->pool.left;
        p_leaf->right = h->pool.right;
        h->pool.left->right = p_leaf;
        h->pool.right->left = p_leaf;
}

struct Leaf *make_leaf(struct Calculator *h, char *tk)
{
        // struct Leaf *leaf = calc_malloc(sizeof(struct Leaf));
        // leaf->value = tk;
        // leaf->left = NULL;
        // leaf->right = NULL;
        return get_free_leaf(NULL, tk);
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

        unsigned char t = get_type(*tk);

        /* checks if is a unary operator */
        if (t == TokenType_OP_ADD || t == TokenType_OP_SUB)
        {
                struct Leaf *right = parse_leaf(h); 
                leaf = make_leaf(tk);
                leaf->right = right;
        }
        else if (t == TokenType_OPEN_PARENT)
        {
                leaf = parse_expr(h, BP_MIN_LIMIT);
                /* consumes close parenthesis */
                get_next(h); 
        }
        else 
                leaf = make_leaf(tk);

        return leaf;
}

struct Leaf *parse_expr(struct Calculator *h, unsigned char  bp)
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

struct Leaf *increasing_prec(struct Calculator *h,struct Leaf *left, unsigned char min_bp)
{
        char next = peek(h);
        unsigned char t = get_type(next);
        unsigned char  bp = get_bp(next);


        if (next == DELIMITER || t == TokenType_CLOSE_PARENT)
                return left;

        if  (t == TokenType_OPEN_PARENT && is_number(*left->value))
        {
                h->tokens->chars[h->tokens->curr][0] = '*';
                next = peek(h);
                t = get_type(next);
                bp = get_bp(next);
        }

        if (is_operator(t))
        {
                while (bp >= min_bp) 
                {
                        char *op = get_next(h);
                        struct Leaf *right = parse_expr(h, bp);
                        left = make_binary_expr(op,left, right);
                        next = peek(h);
                        t = get_type(next);

                        if (next == DELIMITER || t == TokenType_CLOSE_PARENT)
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
                unsigned char curr_t = get_type(*tokens->chars[i]);

                if (i == 0 && (curr_t == TokenType_OP_MUL || curr_t == TokenType_OP_DIV))
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
        assert(tree->value != NULL);

        double lhs = 0.0, rhs = 0.0;
        unsigned char t = get_type(*tree->value);

        if (t == TokenType_NUMBER) 
                return strtod(tree->value, NULL);

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

        printf("%sHead: %s\n", indent, leaf->value ? leaf->value : "NULL");

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

enum Calc_err get_error_code(Calculator *h)
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
		unsigned char *p, *p_next;
        if (h == NULL)
                return;

		// free the heaps
		p = (unsigned char *) h->heap;
		while (p != NULL)
		{
			p_next = (unsigned char *) ((struct CalculatorHeap *) p)->next_heap;
			p = p_next;
		}

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
