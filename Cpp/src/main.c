#include "lexer.h"
#include "parser.h"
#include <cstdio>
#include <iostream>
using namespace std;

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stdout, "Usage: \n");
        fprintf(stdout, "\t%s <FILE_NAME>\n", argv[0]);
        exit(0);
    }
    auto res = read_file(argv[1]);

    Parser parser(res);
    //parser.lexer.print_tokens(parser.tokens);
	auto expr = parser.parseTopLevelStmts();
    for (auto n : expr) {
        n->print();
    }

}
