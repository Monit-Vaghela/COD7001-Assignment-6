#include <stdio.h>
#include <stdlib.h>
#include "parser/parser_driver.h"
#include "ir/ir.h"

extern ast_node *ast_root;
extern void reset_parser_state(void);
extern int yylineno;
extern int parse_error;       
extern int semantic_error;    

int yyparse(void);

int parse_file(const char *filename)
{
    // Reset all state before parsing
    reset_parser_state();
    reset_ir_state();
    yylineno = 1;
    
    yyin = fopen(filename, "r");
    if (!yyin) {
        // perror("open");
        return 1;
    }

    symtab_init();
    yyparse();
    
    if (parse_error || semantic_error) {
        // fprintf(stderr, "\nCompilation failed with errors.\n");
        fclose(yyin);
        return 1;
    }
    
    // printf("\nAST:\n");
    // printf("====\n");
    // ast_print(ast_root, 0);
    
    // printf("\n\nGenerated IR:\n");
    // printf("=============\n");
    
    char *ir_code = generate_ir(ast_root);
    
    // Check if IR generation failed
    if (!ir_code) {
        // fprintf(stderr, "IR generation aborted due to errors.\n");
        fclose(yyin);
        return 1;
    }
    
    // printf("%s\n", ir_code);
    
    // Get line numbers
    int *lines = get_line_numbers();
    int count = get_instruction_count();
    
    // printf("\nLine Number Mapping:\n");
    // printf("====================\n");
    // printf("Array: [");
    // for (int i = 0; i < count; i++) {
    //     printf("%d", lines[i]);
    //     if (i < count - 1) printf(", ");
    // }
    // printf("]\n");
    // printf("Total instructions: %d\n", count);
    
    // Optionally save to file
    // FILE *out = fopen("output.asm", "w");
    // if (out) {
    //     fprintf(out, "%s", ir_code);
    //     fclose(out);
    //     // printf("\nIR saved to: output.asm\n");
    // }
    
    fclose(yyin);
    return 0;
}
