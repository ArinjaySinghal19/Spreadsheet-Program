# Compiler settings
CC = gcc
CFLAGS = -Wall -g

# Source and header files
SRC = main.c
HEADERS = $(wildcard *.h)
EXEC = sheet

# Default target
all: $(EXEC)

# Build the executable
$(EXEC): $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $(SRC)

# Clean generated files
clean:
	rm -f $(EXEC)

.PHONY: all clean #makes sure that makefile treats all and clean as functions, not files to check ma
