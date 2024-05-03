#include "compiler.h"
#include "utils.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void usage(const char *prog_name) {
    printf(" Usage:%s <FILE_NAME>\n\n", prog_name);
    printf(" Options:\n\t"
           "-h,--help 		print help  \n");
}

int main(int argc, char **argv) {
    if (argc < 2) {
        usage(argv[0]);
        exit(1);
    }

    if ((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "--help") == 0)) {
        usage(argv[0]);
        exit(0);
    }

    char *source = compiler::read_file(argv[1]);
	auto file = compiler::new_file(argv[1], source);

    auto tokens = compiler::tokenize(file);
	compiler::print_tokens(tokens);
	utils_free(source);
    return 0;
}
