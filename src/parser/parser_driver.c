#include "parser/parser_driver.h"
#include "ir/ir.h"

int yyparser(void);

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
    generate_ir(ast_root);

    return 0;
}
