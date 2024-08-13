CC = gcc 
OP = -o0
FLAGS = -std=c99 -Wall -Wextra -pedantic -ggdb -Wunused-function -Wmissing-prototypes -Wunreachable-code -Wmissing-declarations -Wshadow -Wcast-align
INSTALL_PATH = /usr/local

# The library
LIBRARY_OBJECTS = calc.o
DYNAMIC_LIBRARY = libcalc.so
STATIC_LIBRARY = libcalc.a

# The executable example
EXAMPLE_SOURCE = main.c
EXAMPLE_BINARY = calc
EXAMPLE_LIBS = -lreadline

all: $(DYNAMIC_LIBRARY) $(STATIC_LIBRARY) $(EXAMPLE_BINARY) $(EXAMPLE_BINARY)_static

$(DYNAMIC_LIBRARY): $(LIBRARY_OBJECTS)
	$(CC) -shared -o $@ $^

$(STATIC_LIBRARY): $(LIBRARY_OBJECTS)
	ar rcs $@ $^

$(EXAMPLE_BINARY): $(EXAMPLE_SOURCE) $(DYNAMIC_LIBRARY)	
	$(CC) $(FLAGS) -o $@ $< -L. -lcalc $(EXAMPLE_LIBS)

$(EXAMPLE_BINARY)_static: $(EXAMPLE_SOURCE) $(STATIC_LIBRARY)
	$(CC) $(FLAGS) -o $@ $< -L. -lcalc $(EXAMPLE_LIBS)	

%.o:%.c calc.h calc_internal.h
	$(CC) $(FLAGS) -c -o $@ $<

install: $(DYNAMIC_LIBRARY) $(STATIC_LIBRARY)
	install -d $(INSTALL_PATH)/lib

clean:
	rm $(EXAMPLE_BINARY) $(OBJECTS) $(DYNAMIC_LIBRARY) $(STATIC_LIBRARY) $(EXAMPLE_BINARY)_static
