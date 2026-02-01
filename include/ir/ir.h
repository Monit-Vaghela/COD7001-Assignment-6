#ifndef IR_H
#define IR_H

#include "parser/ast.h"

char *generate_ir(ast_node *root);
char *get_ir_buf(void);

int *get_line_numbers(void);
int get_instruction_count(void);

ast_node **get_ast_roots(void);

void reset_ir_state(void);

#endif