#ifndef COMPILER_H
#define COMPILER_H
#include "parser/parser_driver.h"
#include "shell/processManager.h"
#include <stdbool.h>

int compile(struct process *proc, bool dg);

#endif /* BUILTINS_H */