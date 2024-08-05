# Command Line Calculator

## Overview
A command line calculator built from scratch that supports direct expression evaluation, an interactive mode, and a history feature. The calculator uses Pratt parsing for expression parsing and features a custom heap and pool for memory management. It can also be used as a library—simply ignore the `main.c` file.

## Features
- **Pratt Parsing**: Efficient parsing technique for arithmetic expressions.
- **Custom Memory Management**: Utilizes a custom heap and memory pool.

## Usage

### Direct Expression Evaluation
To evaluate a single expression directly from the command line:
```bsh
./calc '<expr>'
```

NOTE: Do not forget the quotes!

### Interactive Mode
To enter the interactive mode:
```bsh
./calc
```
In interactive mode, you can input multiple expressions and reference previous results.
