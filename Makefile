# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c11

# Source and output files
SRC = BDD.c
OUT = BDD

# Default target
all: $(OUT)

# Build and Run
$(OUT): $(SRC)
	$(CC) $(CFLAGS) -o $(OUT) $(SRC)
	./$(OUT)
