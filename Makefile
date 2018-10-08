CC = gcc
CFLAGS = -O3
DEP = -lm
SRC = tailog-impl.c
OBJ = tailog-impl.o

all: $(OBJ)
	$(CC) $(CFLAGS) -o tailog $(OBJ) $(DEP)