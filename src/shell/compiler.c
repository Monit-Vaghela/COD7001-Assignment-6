#include "shell/compiler.h"

int compile(const char *filename){
    return parse_file(filename);
}