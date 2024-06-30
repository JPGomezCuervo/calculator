from enum import Enum
import sys
# import pudb; pu.db

# globals
input_str = sys.argv[1]
curr = 0
lex = []

for char in input_str:
    lex.append(char)

class Bp(Enum):
    BP_MIN_LIMIT = 0
    BP_NUMBER = 1
    BP_ADD_SUB = 2
    BP_MUL_DIV = 3


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
        # If leaf node is number, return its integer value
        if self.value.isdigit():
            return int(self.value)

        # Evaluate left and right subtrees recursively
        left_value = self.left.calc()
        right_value = self.right.calc()

        # Perform arithmetic operation based on the operator
        if self.value == '+':
            return left_value + right_value
        elif self.value == '-':
            return left_value - right_value
        elif self.value == '*':
            return left_value * right_value
        elif self.value == '/':
            return left_value / right_value  # Assuming Python 3.x for floating point division


def get_precedence(char):
    if char.isdigit():
        return Bp.BP_NUMBER.value
    elif char == "+" or char == "-":
        return Bp.BP_ADD_SUB.value
    else:
        return Bp.BP_MUL_DIV.value

def is_operator(char):
    return char in ('*', '/', '+', '-')

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
    leaf  = make_node(tk)
    return leaf

# parser

# 2+9*3
def parse_expr(bp = 0):
    left = parse_leaf()

    while True:
        node = increasing_prec(left, bp)
        if node == left:
            break;
        left = node
    
    return left

def increasing_prec(left, bp = Bp.BP_MIN_LIMIT.value):
    next = peek()

    if next == None:
        return left

    if (is_operator(next)):
        while get_precedence(next) > bp:
            op = get_next()
            right = parse_expr(get_precedence(op))
            left = make_binary_expr(left, op, right)
            next = peek()

            if next == None:
                break;

    return left

# print(f"\nFinal expression tree:")
# pudb.set_trace()
lolis = parse_expr()
lolis.visualize()
print(f"result: {lolis.calc()}")


# from enum import Enum
# import sys
#
# # globals
# input_str = sys.argv[1]
# curr = 0
# lex = []
#
# for char in input_str:
#     lex.append(char)
#
# class Bp(Enum):
#     BP_MIN_LIMIT = 0
#     BP_NUMBER = 1
#     BP_UNARY = 2
#     BP_ADD_SUB = 3
#     BP_MUL_DIV = 4
#
# class Leaf:
#     def __init__(self, value):
#         self.value = value
#         self.right = None
#         self.left = None
#
#     def visualize(self, indent=""):
#         print(f"{indent}Head: {self.value}")
#
#         if self.left:
#             print(f"{indent}Left:")
#             self.left.visualize(indent + "    ")
#
#         if self.right:
#             print(f"{indent}Right:")
#             self.right.visualize(indent + "    ")
#
#     def calc(self):
#         # If leaf node is number, return its integer value
#         if self.value.isdigit() or (self.value.startswith('-') and self.value[1:].isdigit()):
#             return int(self.value)
#
#         # Evaluate left and right subtrees recursively
#         left_value = self.left.calc() if self.left else 0
#         right_value = self.right.calc() if self.right else 0
#
#         # Perform arithmetic operation based on the operator
#         if self.value == '+':
#             return left_value + right_value
#         elif self.value == '-':
#             return left_value - right_value
#         elif self.value == '*':
#             return left_value * right_value
#         elif self.value == '/':
#             return left_value / right_value  # Assuming Python 3.x for floating point division
#         elif self.value == 'NEG':
#             return -right_value
#
# def get_precedence(char):
#     if char.isdigit() or (char == '-' and (curr == 0 or lex[curr-1] in '*/+-(')):
#         return Bp.BP_NUMBER.value
#     elif char in "+-":
#         return Bp.BP_ADD_SUB.value
#     elif char in "*/":
#         return Bp.BP_MUL_DIV.value
#     else:
#         return Bp.BP_MIN_LIMIT.value
#
# def is_operator(char):
#     return char in ('*', '/', '+', '-')
#
# # utils
# def get_token():
#     return lex[curr]
#
# def get_next():
#     global curr
#     tk = None
#     if curr < len(lex):
#         tk = lex[curr]
#         curr += 1
#     return tk
#
# def peek():
#     tk = None
#     if curr < len(lex):
#         tk = lex[curr]
#     return tk
#
# # make nodes
# def make_node(char):
#     leaf = Leaf(char)
#     leaf.left = None
#     leaf.right = None
#     return leaf
#
# def make_binary_expr(left, op, right):
#     leaf = Leaf(op)
#     leaf.left = left
#     leaf.right = right
#     return leaf
#
# def parse_leaf():
#     tk = get_next()
#     if tk is None:
#         return None
#
#     if tk == '-':
#         # Handle unary minus
#         right = parse_leaf()
#         leaf = Leaf('NEG')
#         leaf.right = right
#         return leaf
#     else:
#         return make_node(tk)
#
# # parser
#
# # 2+9*3 or -1+2
# def parse_expr(bp=Bp.BP_MIN_LIMIT.value):
#     left = parse_leaf()
#     if left is None:
#         return None
#
#     while True:
#         next_token = peek()
#         if next_token is None or not is_operator(next_token) or get_precedence(next_token) <= bp:
#             break
#
#         get_next()  # Consume operator
#         right = parse_expr(get_precedence(next_token))
#         if right is None:
#             break
#
#         left = make_binary_expr(left, next_token, right)
#     
#     return left
#
# # Testing the parser
# lolis = parse_expr()
# if lolis is not None:
#     lolis.visualize()
#     print(f"Result: {lolis.calc()}")
# else:
#     print("Failed to parse the expression.")
