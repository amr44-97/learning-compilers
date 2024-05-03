CC := gcc

CFLAGS := -Wall  -g -std=c23 -pedantic

OBJ := tokenizer.o

SRC := src/main.c

BIN_DIR := bin
EXE = compile

.PHONY: build 

#build: src/main.c src/tokenizer.h src/tokenizer.c src/parser.h src/parser.c 
#	$(CC) $(CFLAGS) tokenizer.c $(SRC) -o $(BIN_DIR)/$(EXE)

build: $(wildcard src/*.c)
	$(CC) $(CFLAGS) src/tokenizer.c src/utils.c $(SRC) -o $(BIN_DIR)/$(EXE)

clean:
	rm  $(BIN_DIR)/$(EXE)

