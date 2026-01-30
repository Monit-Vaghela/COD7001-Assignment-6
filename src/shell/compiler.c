#include "shell/compiler.h"
#include "shell/processManager.h"
#include "ir/ir.h"
#include "parser/ast.h"
#include "assembler/assembler.h"

int compile(struct process *proc){
    // first letd parse and get the ast.
    // printf("1\n");
    if(parse_file(proc->program_path) != 0){
        return 0;
    }
    proc->ast= ast_root;
    ast_print(proc->ast, 0);
    // printf("2\n");
    if(generate_ir(proc->ast) != 0){
        return 0;
    }
    proc->ir = get_ir_buf();
    // printf("This is done\n");
    FILE *out = fopen("log/output.asm", "w");
    if (out) {
        fprintf(out, "%s", proc->ir);
        fclose(out);
        // printf("\nIR saved to: output.asm\n");
    }
    
    // initialize the vm
    unsigned char *code = NULL;
    int res = assemble(proc->ir, &code);
    // printf("DONE ASSEMBLING, %d\n", res);
    if(res == -1){
        return 0;
    }
    // print the program code
    printf("Bytecode (%d bytes):\n", res);
    for (int i = 0; i < res; i++) {
        printf("%d -> %02X \n",i ,code[i]);
    }
    printf("\n");

    // allocate memory for VM
    proc->vm = malloc(sizeof(vm));

    // initialize the vm
    vm_init(proc->vm, code, res);

    // run vm
    vm_run(proc->vm, 0, 0);

    return 0;
}