#include <stdio.h>
#include "parser/ast.h"
#include "parser/symbol_table.h"
#include "ir/ir.h"

extern ast_node *ast_root;
int parse_file(const char *filename);

int main(int argc, char **argv)
{
    if (argc != 2) {
        printf("usage: %s <input>\n", argv[0]);
        return 1;
    }

    symtab_init();
    parse_file(argv[1]);
    generate_ir(ast_root);

    printf("ASM generated → out.asm\n");
    return 0;
}
