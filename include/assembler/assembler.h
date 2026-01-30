#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "VM/instruction.h"

int assemble(const char *src, unsigned char **out_code);

#endif
