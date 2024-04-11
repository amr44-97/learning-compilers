#include "tokenizer.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>

auto read_file_to_string(const char *file_name) -> std::string * {
  FILE *fp = fopen(file_name, "r");
  fseek(fp, 0, SEEK_END);
  auto file_size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  char *bptr = (char *)malloc(file_size + 1);
  fread(bptr, file_size, 1, fp);
  std::string *buffer = new std::string(bptr);
  free(bptr);
  fclose(fp);
  return buffer;
}

auto usage(const char *prog_name) -> void {
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

  auto source = read_file_to_string(argv[1]);

  Tokenizer tk = Tokenizer::Init(source);
  auto token = tk.next();
  while (true) {
    const auto tok_buf =
        tk.token_buf(token);
    printf("%s : `%s`\n", token.to_str().c_str(), tok_buf.c_str());
    if (token.type == TokenType::Eof)
      break;
    token = tk.next();
  }
  
  return 0;
}
