CC = clang++

CFLAGS = -Wall -pedantic 
OBJ = tokenizer.o

.PHONY: tokenizer.o  compile 

compile: main.cpp tokenizer.h tokenizer.cpp parser.h parser.cpp 
	$(CC) $(CFLAGS) $(OBJ) $(SRC) -o compile

tokenizer.o: tokenizer.h tokenizer.cpp 
	$(CC) -c tokenizer.cpp


