# Makefile for Snake Game

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -std=c99

# Linker flags
LDFLAGS = -lncurses

# Target executable
TARGET = snake

# Source file
SRC = snake.c

# Object file
OBJ = $(SRC:.c=.o)

# Default target
all: $(TARGET)

# Linking the target executable
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

# Compiling source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up object files
clean:
	rm -f $(OBJ)

# Clean up object files and executable
mrproper: clean
	rm -f $(TARGET)

# Rebuild everything from scratch
re: mrproper all

# Phony targets (targets that don't represent files)
.PHONY: all clean mrproper re
