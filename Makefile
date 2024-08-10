CC = gcc 
OP =  -o0
FLAGS = -std=c99 -Wall -Wextra -pedantic -ggdb -Wunused-function -Wmissing-prototypes -Wunreachable-code -Wmissing-declarations -Wshadow -Wcast-align
OBJECTS = main.o calc.o
CFILES = main.c calc.c
BINARY = calc
LIBS = -lreadline
INSTALL_PATH = /usr/local/bin

all: $(BINARY)

$(BINARY): $(OBJECTS)
	$(CC) $(FLAGS) -o $@ $^ $(LIBS)

%.o:%.c calc.h calc_internal.h
	$(CC) $(FLAGS) -c -o $@ $<

install: $(BINARY)
	install -m 0755 $(BINARY) $(INSTALL_PATH)

clean:
	rm $(BINARY) $(OBJECTS)
