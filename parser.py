import sys
from enum import Enum

# globals
input_str = sys.argv[1]
curr = 0
lex = []

for char in input_str:
    lex.append(char)

class Bp(Enum):
    BP_MIN_LIMIT = 0
    BP_NUMBER = 1
    BP_ADD_SUB = 3
    BP_MUL_DIV = 4


class Leaf:
    def __init__(self, value):
        self.value = value
        self.right = None
        self.left = None

    def visualize(self, indent=""):
        print(f"{indent}Head: {self.value}")

        if self.left:
            print(f"{indent}Left:")
            self.left.visualize(indent + "    ")

        if self.right:
            print(f"{indent}Right:")
            self.right.visualize(indent + "    ")

    def calc(self):
        if self.value.isdigit(): 
            return int(self.value)

        if self.left is not None:
            left_value = self.left.calc()
        else:
            left_value = 0

        if self.right is not None:
            right_value = self.right.calc()
        else:
            right_value = 0

        if self.value == '+':
            return left_value + right_value
        elif self.value == '-':
            return left_value - right_value
        elif self.value == '*':
            return left_value * right_value
        elif self.value == '/':
            return left_value / right_value
        elif self.value == "NEG":
            return  -right_value


def get_precedence(char):
    if char.isdigit() or (char == '-' and curr == 0):
        return Bp.BP_NUMBER.value
    elif char == '+' or char == '-':
        return Bp.BP_ADD_SUB.value
    else:
        return Bp.BP_MUL_DIV.value

def is_operator(char):
    return char in ('*', '/', '+', '-')

def is_open_parenthesis(char):
    return char == '('

def is_close_parenthesis(char):
    return char == ')'

# utils
def get_token():
    return lex[curr]

def get_next():
    global curr
    tk = None
    if curr < len(lex):
        tk = lex[curr]
        curr += 1
    return tk

def peek():
    tk = None
    if curr < len(lex):
        tk = lex[curr]
    return tk

# make nodes
def make_node(char):
    leaf = Leaf(char)
    leaf.left = None
    leaf.right = None
    return leaf

def make_binary_expr(left, op, right):
    leaf = Leaf(op)
    leaf.left = left
    leaf.right = right

    return leaf

def parse_leaf():
    tk = get_next()

    if tk == '-':
        right = parse_leaf()
        left = Leaf('NEG')
        left.right = right
        return left
    elif is_open_parenthesis(tk):
        left = parse_expr()
        get_next()
        return left
    else:
        return  make_node(tk)

# cuando la precedencia aumenta el arbol se construye hacia abajo
# cuando la precedencia disminuye el arbol se construye hacia arriba
# parser
 #     _
 #    / \
 #   +   1
 #  / \
 # 3   *
 #    / \
 #   5   2

def parse_expr(bp = 0):
    left = parse_leaf()

    while True:
        node = increasing_prec(left, bp)
        if left == node:
            break;
        left = node
    
    return left

def increasing_prec(left, bp = Bp.BP_MIN_LIMIT.value):
    next = peek()

    if is_close_parenthesis(next):
        return left

    if next and is_operator(next):
        while get_precedence(next) > bp:
            op = get_next()
            right = parse_expr(get_precedence(op))
            left = make_binary_expr(left, op, right)
            next = peek()

            if next == None or is_close_parenthesis(next):
                break;


    return left

# print(f"\nFinal expression tree:")
lolis = parse_expr()
lolis.visualize()
print(f"result: {lolis.calc()}")