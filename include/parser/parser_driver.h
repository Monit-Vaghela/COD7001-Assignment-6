#ifndef PARSER_DRIVER_H
#define PARSER_DRIVER_H
#include <stdio.h>
#include "parser/ast.h"
#include "parser/symbol_table.h"

extern int yyparse(void);
extern FILE *yyin;
extern  ast_node *ast_root;

int parse_file(const char *filename);

#endif 