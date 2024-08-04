CC = gcc
OP =  -o0
FLAGS = -Wall -Wextra -pedantic -ggdb -Wunused-function -Wmissing-prototypes -Wunreachable-code -Wmissing-declarations -Wshadow -Wcast-align
OBJECTS = main.o calc.o
CFILES = main.c calc.c
BINARY = calc
LIBS = -lreadline

all: $(BINARY)

$(BINARY): $(OBJECTS)
	$(CC) $(FLAGS) -o $@ $^ $(LIBS)

%.o:%.c calc.h calc_internal.h
	$(CC) $(FLAGS) -c -o $@ $<

clean:
	rm $(BINARY) $(OBJECTS)
