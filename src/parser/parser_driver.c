#include "parser/parser_driver.h"
#include "ir/ir.h"
#include <stdlib.h>

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
    
    printf("\nAST:\n");
    printf("====\n");
    ast_print(ast_root, 0);
    
    printf("\n\nGenerated IR:\n");
    printf("=============\n");
    char *ir_code = generate_ir(ast_root);
    printf("%s\n", ir_code);
    
    // Optionally save to file
    FILE *out = fopen("output.asm", "w");
    if (out) {
        fprintf(out, "%s", ir_code);
        fclose(out);
        printf("\nIR saved to: output.asm\n");
    }
    
    free(ir_code);
    fclose(yyin);
    generate_ir(ast_root);

    return 0;
}