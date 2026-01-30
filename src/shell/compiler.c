#include "shell/compiler.h"
#include "shell/processManager.h"
#include "ir/ir.h"
#include "parser/ast.h"

int compile(struct process *proc){
    // first letd parse and get the ast.
    // printf("1\n");
    if(parse_file(proc->program_path) != 0){
        return 0;
    }
    proc->ast= ast_root;
    // ast_print(proc->ast, 0);
    // printf("2\n");
    proc->ir = generate_ir(proc->ast);
    // printf("This is done\n");
    // FILE *out = fopen("log/output.asm", "w");
    // if (out) {
    //     fprintf(out, "%s", proc->ir);
    //     fclose(out);
    //     printf("\nIR saved to: output.asm\n");
    // }
    
    return 0;
}