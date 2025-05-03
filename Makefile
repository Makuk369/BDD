NAME = BDD

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c11

# Source and output files
SRC = $(NAME).c

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = -o $(NAME)

# Default target
# Build and Run
all:
	$(CC) $(SRC) $(CFLAGS) $(OBJ_NAME)

buildAndRun:
	$(CC) $(SRC) $(CFLAGS) $(OBJ_NAME) 
	./$(NAME).exe
