CC := clang++

CFLAGS := -Wall -std=c++23 

OBJ := tokenizer.o

SRC := src/main.cpp

BIN_DIR := bin
EXE = compiler-v1

.PHONY: build 

#build: src/main.cpp src/tokenizer.h src/tokenizer.cpp src/parser.h src/parser.cpp 
#	$(CC) $(CFLAGS) tokenizer.cpp $(SRC) -o $(BIN_DIR)/$(EXE)

build: $(wildcard src/*.cpp)
	$(CC) $(CFLAGS) src/tokenizer.cpp $(SRC) -o $(BIN_DIR)/$(EXE)

clean:
	rm  $(BIN_DIR)/$(EXE)

