CC = gcc
OP =  -o0
FLAGS = -Wall -Wextra -pedantic -ggdb -Wunused-function -Wmissing-prototypes
OBJECTS = main.o calc.o
CFILES = main.c calc.c
BINARY = calc

all: $(BINARY)

$(BINARY): $(OBJECTS)
	$(CC) $(FLAGS) -o $@ $^

%.o:%.c calc.h calc_internal.h
	$(CC) $(FLAGS) -c -o $@ $<

