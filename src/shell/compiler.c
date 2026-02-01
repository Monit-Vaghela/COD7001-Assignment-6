#include "shell/compiler.h"
#include "shell/processManager.h"
#include "ir/ir.h"
#include "parser/ast.h"
#include "assembler/assembler.h"
#include <stdbool.h>
#include <debugger/entry.h>
#include "VM/instruction.h"

static int get_bytes(unsigned char opcode) {
    switch (opcode) {
        case PUSH:
        case STORE:
        case LOAD:
        case JMP:
        case JZ:
        case JNZ:
        case CALL:
            return 5;

        default:
            return 1;
    }
}


int compile(struct process *proc){
    if(proc->state != PROC_TERMINATED){
        proc->state = PROC_RUNNING;
        // first letd parse and get the ast.
        // printf("Process State %d\n", proc->state);
        if(parse_file(proc->program_path) != 0){
            proc->exit_code = 1;
            return 0;
        }
        proc->ast= ast_root;
        // ast_print(proc->ast, 0);
        // printf("2\n");
        proc->ir = generate_ir(ast_root);
        if (!proc->ir) {
            fprintf(stderr, "IR generation aborted due to errors.\n");
            fclose(yyin);
            proc->exit_code = 2;
            return 0;
        }

        // printf("2\n");
        int *line_numbers = get_line_numbers();
        // printf("2\n");
        int instruction_count = get_instruction_count();
        proc->lines = get_last_line();
        // printf("2\n");
        // for(int i = 0; i < proc->dbg_info->instruction_count; i++){
        //     printf("%d -> %d\n", i, proc->dbg_info->line_numbers[i]);
        // }
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
            proc->exit_code = 4;
            return 0;
        }
        // print the program code
        // printf("Bytecode (%d bytes):\n", res);
        // for (int i = 0; i < res; i++) {
        //     printf("%d -> %02X \n",i ,code[i]);
        // }
        // printf("\n");

        // int *debug_info = get_debug_info();
        // int debug_size = get_debug_size();
        // for(int i = 0; i < debug_size; i++){
        //     printf("%d -> %d\n", i, debug_info[i]);
        // }
        // allocate memory for VM
        proc->map = malloc(sizeof(source_mapping) * instruction_count);
        proc->map_count = instruction_count;
        int pc = 0;
        for(int i = 0; i < instruction_count; i++){
            int bytes = get_bytes(code[pc]);
            proc->map[i].instruction = opcode_to_string(code[pc]);
            proc->map[i].instruction_start = pc;
            proc->map[i].lineno = line_numbers[i];
            pc+=bytes;
            
        }
        proc->max_pc = pc - 1;
        // print_source_mappings(proc);
        proc->vm = malloc(sizeof(vm));
        // initialize the vm
        vm_init(proc->vm, code, res);
    }
    proc->state = PROC_SUBMITTED;
    proc->exit_code = 0;
    return 0;
}

int run(struct process *proc, bool dg){
    if(proc->exit_code != 0){
        printf("Cannot run this program due to ");
        if(proc->exit_code == 1){
            printf("PARSER Error ");
        }
        else if(proc->exit_code == 2){
            printf("IR Error ");
        }
        else if(proc->exit_code == 4){
            printf("ASSEMBLER Error ");
        }
        printf("during compilation\n");
        return 0;
    }
    if(dg){
        debug(proc);
        proc->state = PROC_TERMINATED;
        return 0;
    }
    vm_reset(proc->vm);
    // run vm
    int vm_res = vm_run(proc->vm, 0, 0, 1);
    proc->state = PROC_TERMINATED;

    proc->exit_code = vm_res;
    return 0;
}