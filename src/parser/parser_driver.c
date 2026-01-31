#include <stdio.h>
#include <stdlib.h>
#include "parser/parser_driver.h"
#include "ir/ir.h"

extern ast_node *ast_root;
extern void reset_parser_state(void);
extern int yylineno;  // ← Add this declaration

int yyparse(void);

int parse_file(const char *filename)
{
    // Reset all state before parsing
    reset_parser_state();
    reset_ir_state();
    yylineno = 1;  // ← Reset line number counter!
    
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
    
    generate_ir(ast_root);
    char *ir_code = get_ir_buf();
    
    printf("%s\n", ir_code);
    
    // Get and print line number array
    int *lines = get_line_numbers();
    int count = get_instruction_count();
    
    printf("\nLine Number Mapping:\n");
    printf("====================\n");
    printf("Array: [");
    for (int i = 0; i < count; i++) {
        printf("%d", lines[i]);
        if (i < count - 1) printf(", ");
    }
    printf("]\n");
    printf("Total instructions: %d\n", count);
    
    // Optionally save to file
    FILE *out = fopen("output.asm", "w");
    if (out) {
        fprintf(out, "%s", ir_code);
        fclose(out);
        printf("\nIR saved to: output.asm\n");
    }
    
    fclose(yyin);
    return 0;
}