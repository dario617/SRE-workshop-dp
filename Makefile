CC = gcc
CFLAGS = -O3 -lm
SRC = tailog-impl.c
OBJ = tailog-impl.o

all: $(OBJ)
	$(CC) $(CFLAGS) -o tailog $(OBJ)