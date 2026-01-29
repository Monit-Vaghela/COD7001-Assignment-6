#include <stdio.h>
#include "ast.h"
#include "symbol_table.h"

extern int yyparse(void);
extern FILE *yyin;

extern  ast_node *ast_root;

int parse_file(const char *filename)
{
    yyin = fopen(filename, "r");
    if (!yyin) {
        perror("open");
        return 1;
    }

    symtab_init();
    yyparse();
    ast_print(ast_root, 0);
    fclose(yyin);

    return 0;
}
