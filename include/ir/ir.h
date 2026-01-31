#ifndef IR_H
#define IR_H

#include "parser/ast.h"

int generate_ir(ast_node *root);
char *get_ir_buf();
int *get_debug_info();
int get_debug_size();

#endif