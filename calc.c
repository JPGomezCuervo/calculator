#include "calc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

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

int calc_scan()
{
        char c;
        size_t pos = 0;
        size_t buff_size = 50;
        enum Type t;

        if (input == NULL)
        {
                input = calc_malloc(buff_size * sizeof(char));
                input[buff_size - 1] = '\0';
        }

        while ((c = fgetc(stdin)) != EOF && c != '\n')
        {

                if (pos >= buff_size)
                {
                        buff_size *= 2;
                        input = calc_realloc(input, buff_size);
                }

                if (!isspace(c))
                {
                        if ((t = get_type(c)) == UNKNOWN || t == LIMIT)
                        {
                                flush_stdin();
                                dead(ERR_UNKNOWN_OPERATOR);
                                return 0;
                        }

                        input[pos] = c;
                        pos++;
                }

        }

        if (feof(stdin))
        {
                clearerr(stdin);
                printf("\n");
                exit(0);
        }

        input[pos] = '?';
        pos++;

        input[pos] = '\0';

        return pos;
}

void flush_stdin(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
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
                /*Reverse loop to take advantage of the Zero Flag cpu optmization*/
                for (int i = (int)tokens->len - 1; i >= 0; i--)
                        free(tokens->chars[i]);

                free(tokens->chars);
                free(tokens);
                tokens = NULL;
        }


        if (input) 
        {
                free(input);
                input = NULL;
        }

        if (tree)
        {
                free_tree(tree);
                tree = NULL;
        }
}

void add_token(size_t *i, enum Type t, size_t input_len, size_t tokens_pos) 
{
        if (t == NUMBER)
        {
                size_t size = 0;
                const char *p_input = &input[*i];

                while ((isdigit(*p_input) || *p_input == '.') && *i < input_len)
                {
                        size++;
                        p_input++;
                }

                char *str = calc_malloc(sizeof(char) * (size + 1));

                for (size_t j = 0; j < size; j++)
                {
                        str[j] = input[*i];
                        (*i)++;
                }

                str[size] = '\0';
                tokens->chars[tokens_pos] = str;
                (*i)--;  // Adjust index since the loop will increment i once more
        }
        else
        {
                tokens->chars[tokens_pos] = calc_malloc(sizeof(char) * 2);
                tokens->chars[tokens_pos][0] = input[*i];
                tokens->chars[tokens_pos][1] = '\0';
        }
}

char *get_next()
{
        assert(tokens != NULL);

        char *pc = NULL;
        if (*tokens->chars[tokens->curr] != DELIMITER)
        {
                pc = tokens->chars[tokens->curr];
                tokens->curr++;
        }

        return pc;
}

char peek()
{
        assert(tokens != NULL);

        if (*tokens->chars[tokens->curr] != DELIMITER)
                return *tokens->chars[tokens->curr];

        return DELIMITER;
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

struct Leaf *parse_leaf()
{
        char *tk = get_next();
        struct Leaf *leaf = NULL;

        if (tk == NULL)
                return leaf;

        enum Type t = get_type(*tk);

        if (t == OP_ADD || t == OP_SUB)
        {
                struct Leaf *right = parse_leaf(); 
                leaf = make_leaf(tk);
                leaf->right = right;
        }
        else if (t == OPEN_PARENT)
        {
                leaf = parse_expr(MIN_LIMIT);
                /* consumes close parenthesis */
                get_next(); 
        }
        else 
        {
                leaf = make_leaf(tk);
        }

        return leaf;
}

struct Leaf *parse_expr(enum Bp bp)
{
        struct Leaf *left = parse_leaf();
        struct Leaf *node = NULL;

        while (true)
        {
                node = increasing_prec(left, bp);

                if (left == node)
                        break;
                left = node;
        }
        return left;
}

struct Leaf *increasing_prec(struct Leaf *left, enum Bp min_bp)
{
        char next = peek();
        enum Type t = get_type(next);
        enum Bp bp = get_bp(next);


        if (next == DELIMITER || t == CLOSE_PARENT)
                return left;

        if (is_operator(t))
        {
                while (bp >= min_bp) 
                {
                        char *op = get_next();
                        struct Leaf *right = parse_expr(bp);
                        left = make_binary_expr(op,left, right);
                        next = peek();
                        t = get_type(next);

                        if (next == DELIMITER || t == CLOSE_PARENT)
                                break;
                }
        }
        return left;
}

double eval_tree(struct Leaf *tree)
{
        assert(tree != NULL);
        assert(tree->value != NULL);

        double lhs = 0.0, rhs = 0.0;
        enum Type t = get_type(*tree->value);

        if (t == NUMBER) 
                return strtod(tree->value, NULL);


        lhs = tree->left != NULL ? eval_tree(tree->left) : 0;
        rhs = tree->right != NULL ? eval_tree(tree->right) : 0;


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
                                dead(ERR_DIVIDE_BY_ZERO);

                        return lhs / rhs;
                case UNARY_NEG:
                        return -rhs;
                default:
                        dead(ERR_UNKNOWN_OPERATOR);
                        return 0.0;
        }
}

void dead(enum Calc_err err)
{
        assert(err >= 0);

        fprintf(stderr, "ERROR: ");
        fprintf(stderr, "%s\n", calc_err_msg[err]);

        if (continuous_mode)
                calc_cleanup();
        else 
                exit(err);
}

bool check_semantics()
{
        assert(tokens->chars != NULL);

        bool was_operator = false;
        bool everything_ok = true;

        for (size_t i = 0; i < tokens->len; i++) 
        {
                enum Type curr_t = get_type(*tokens->chars[i]);

                if (i == 0 && (curr_t == OP_MUL || curr_t == OP_DIV)) 
                {
                        everything_ok = false;
                        break;
                }

                if (isdigit(*tokens->chars[i]) || *tokens->chars[i] == '.') 
                {
                        was_operator = false;
                        continue;
                }

                if (is_operator(curr_t) && was_operator) 
                {
                        everything_ok = false;
                        break;
                }

                if (curr_t == LIMIT && was_operator) 
                {
                        everything_ok = false;
                        break;
                }

                if (curr_t == CLOSE_PARENT && was_operator)
                {
                        everything_ok = false;
                        break;
                }

                was_operator = is_operator(curr_t);
        }

        if (!everything_ok) 
                dead(ERR_SYNTAX);

        return everything_ok;
}

int make_tokens()
{
        int tks_readed = 0;
        for (size_t i = 0; i < input_len; i++)
        {
                enum Type t = get_type(input[i]);
                add_token(&i, t, input_len, tks_readed);
                tks_readed++;
        }
        tokens->chars = calc_realloc(tokens->chars, sizeof(char*) * tks_readed);
        tokens->len = tks_readed;

        return tks_readed;
}

struct Lexer *initialize_tokens(size_t input_len)
{
        if(tokens != NULL)
        {
                for (int i = (int)tokens->len - 1; i >= 0; i--)
                        free(tokens->chars[i]);

                free(tokens->chars);
                free(tokens);
        }

        tokens = calc_malloc(sizeof(struct Lexer));
        tokens->chars = calc_malloc(sizeof(char*) * input_len);
        tokens->curr = 0;

        return tokens;
}
