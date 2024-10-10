#pragma once


#define Color_Black "\x1b[30m"
#define Color_Red "\x1b[31m"
#define Color_Green "\x1b[32m"
#define Color_Yellow "\x1b[33m"
#define Color_Blue "\x1b[34m"
#define Color_Magenta "\x1b[35m"
#define Color_Cyan "\x1b[36m"
#define Color_White "\x1b[37m"
#define Color_Bright_black "\x1b[90m"
#define Color_Bright_red "\x1b[91m"
#define Color_Bright_green "\x1b[92m"
#define Color_Bright_yellow "\x1b[93m"
#define Color_Bright_blue "\x1b[94m"
#define Color_Bright_magenta "\x1b[95m"
#define Color_Bright_cyan "\x1b[96m"
#define Color_Bright_white "\x1b[97m"
#define Color_Bold "\x1b[1m"
#define Color_Dim "\x1b[2m"
#define Color_Reset "\x1b[0m"

#include <stdlib.h>

void* utils_alloc(size_t size);
void utils_free_all();
void utils_free(void* ptr);
