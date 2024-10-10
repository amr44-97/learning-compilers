#pragma once
#include "type.h"
#include <map>
#include <stdint.h>
#include <string>
#include <vector>
using std::vector, std::string, std::map;

struct Symbol {
    string name;
    Type type;

    union {
        uint64_t Integer;
        long double Double;
        float Float;
        string str;
    } value;
};
