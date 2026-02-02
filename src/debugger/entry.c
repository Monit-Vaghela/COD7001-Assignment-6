#include "debugger/entry.h"
#include "VM/stack.h"
#include "debugger/breakpoints.h"
#include "VM/vm.h"

static int allow_break(struct process *proc, int instruction_line){
    unsigned char opcode = proc->vm->code[instruction_line];
    switch (opcode) {
        case PUSH: return 0;
        case POP: return 0;

        case STORE: return 0;
        case LOAD:  return 0;

        case PRINT: return 0;
        default: return 1;
    }
}

int debug(struct process *proc) {
    // printf("Ready to debug program: %s \n", argv[1]);
    char cmd[256];

    // firstly lets create debug info
    struct debug_frame info;
    info.state = LOADED;
    info.breakpoint_count = 0;

    signal(SIGINT, SIG_IGN);

    while(1)
    {
        printf(COLOR_GREEN "dbg> " COLOR_RESET);

        if (!fgets(cmd,sizeof(cmd),stdin)){ // For EOF and input failure handling
            printf("\n");
            break;
        }

        cmd[strcspn(cmd, "\n")] = 0;   // Handling newline character
        
        if(strcmp(cmd, "quit") == 0 || strcmp(cmd, "q") == 0)
        {
            int exit = debugger_quit();
            if(exit)
            {
                break;
            }
            continue;
        }

        else if(strcmp(cmd, "run") == 0 || strcmp(cmd, "r") == 0)
        {
            // first need to reset the vm;
            vm_reset(proc->vm);
            init_breakpoints(&info, proc->vm->code);
            int res = debugger_run(proc, &info);
            if(res == 0) {
                print_info("Code execution complete");
                info.state = EXITED;
            }
            if(res == 101){
                print_info("BreakPoint Hit");
                if(handle_breakpoint(proc, &info) != 0){
                    print_error("Encountered Unknown Error\n");
                    info.state = EXITED;
                    return 8;
                }
                info.state = RUNNING;
                int pc = proc->vm->pc;
                // single Stepping
                vm_run(proc->vm, 0, 0, -1);
                info.state = STOPPED;
                //add break points back
                // printf("SETTING BACK THE BREAK at %d\n", pc);
                proc->vm->code[pc] = 0xCC;
            }
        }

        else if(strcmp(cmd, "step") == 0 || strcmp(cmd, "s") == 0)
        {
            int res = debugger_step(proc, &info);
            if(res == 0) {
                print_info("Code execution complete");
                info.state = EXITED;
            }
            if(res == 101){
                if(handle_breakpoint(proc, &info) != 0){
                    print_error("Encountered Unknown Error\n");
                    info.state = EXITED;
                    return 8;
                }
                info.state = RUNNING;
                vm_run(proc->vm, 0, 0, -1);
                info.state = STOPPED;
                // we wont be doing any single stepping as it is not required for my architecture.
            }
            else if(res == 102){
                info.state = STOPPED;
            }
        }
        else if(strcmp(cmd, "continue") == 0 || strcmp(cmd, "c") == 0)
        {
            int res = debugger_continue(proc, &info);
            if(res == 0) {
                print_info("Code execution complete");
                info.state = EXITED;
            }
            if(res == 101){
                print_info("BreakPoint Hit");
                if(handle_breakpoint(proc, &info) != 0){
                    print_error("Encountered Unknown Error\n");
                    info.state = EXITED;
                    return 8;
                }
                info.state = RUNNING;
                int pc = proc->vm->pc;
                // single Stepping
                vm_run(proc->vm, 0, 0, -1);
                info.state = STOPPED;
                //add break points back
                // printf("SETTING BACK THE BREAK at %d\n", pc);
                proc->vm->code[pc] = 0xCC;
            }
        }
        
        else if(strcmp(cmd, "bpinfo") == 0)
        {
            breakpoint_info(&info);
        }
        else if(strncmp(cmd, "disablebp ", 10) == 0)
        {
            int lineno = strtoul(cmd + 10, NULL, 0);
            int res = disable_breakpoints(&info, lineno, proc->vm->code);
            if (res == 0) {
                printf(COLOR_ORANGE
                "[DEBUGGER / Info] : Breakpoint disabled at "
                VAL_BLUE("%d")
                COLOR_RESET "\n",
                lineno);
            }
        }
        else if(strncmp(cmd, "enablebp ", 9) == 0)
        {
            int lineno = strtoul(cmd + 9, NULL, 0);
            int res = enable_breakpoints(&info, lineno, proc->vm->code);
            if (res == 0) {
                printf(COLOR_ORANGE
                "[DEBUGGER / Info] : Breakpoint enabled at "
                VAL_BLUE("%d")
                COLOR_RESET "\n",
                lineno);
            }
        }
        else if(strcmp(cmd, "state") == 0)
        {
            printf(COLOR_ORANGE
                "[DEBUGGER / Info] : Current state of debugger is "
                VAL_BLUE("%s")
                COLOR_RESET "\n",
                debugger_state_to_string(&info)
            );
        }
        else if(strncmp(cmd, "break ", 6) == 0)
        {
            int lineno = (int)strtoul(cmd + 6, NULL, 0);
            if(lineno > proc->max_pc){
                print_error("Address our of bounds");
                continue;
            }
            if(allow_break(proc, lineno) != 0){
                print_error("Break points are only allowed on certain instructions (try PUSH POP LOAD STORE PRINT)\n");
                continue;
            }
            // Only allow break points on PUSH, POP, LOAD, STORE
            int res = set_breakpoint(&info, lineno, proc->vm->code, 0);
            if (res == 0) {
                printf(COLOR_ORANGE
                "[DEBUGGER / Info] : Breakpoint set at "
                VAL_BLUE("0x%d")
                COLOR_RESET "\n",
                lineno);
            }
        }
        else if (strncmp(cmd, "mem", 4) == 0) {
            print_mem(proc->vm);
        }
        else if(strncmp(cmd, "vm_state", 8) == 0) {
            print_vm_state(proc->vm, proc->vm->code[proc->vm->pc]);
        }
        else if(strncmp(cmd, "memstat", 7) == 0) {
            heap_print(proc->vm->heap_head);
        }
        else if(strcmp(cmd, "regs") == 0){
            printf("Stack Pointer -> %d\n", proc->vm->stack.top);
            printf("Program Counter -> %d\n", proc->vm->pc);
        }
        else if(strcmp(cmd, "stack") == 0){
            stack_print(&(proc->vm->stack));
        }
        else if(strcmp(cmd, "break_code") == 0){
            print_source_mappings(proc);
        }
        else if (strcmp(cmd, "help") == 0) {
            print_info("Available debugger commands:");

            print_info("run | r                  : Start the program");
            print_info("continue | c             : Continue execution");
            print_info("step | s                 : Execute one instruction");
            print_info("quit | q                 : Quit the debugger");

            print_info("state                    : Show current debugger state");

            print_info("regs                     : Show general purpose registers");

            print_info("break <addr>             : Set breakpoint at address");
            print_info("disablebp <addr>         : Disable breakpoint");
            print_info("enablebp <addr>          : Enable breakpoint");
            print_info("bpinfo                   : List all breakpoints");

            print_info("mem                      : Prints the data store in intialized memories");
            print_info("break_code               : Prints source code mappings");
            print_info("stack                    : Prints the stack");
            print_info("memstat                  : Prints the Heap");
            print_info("vm_state                 : Prints the current snapshot to vm");
            print_info("help                     : Show this help message");
        }
        else {
            print_info("Command does not exist. Type 'help' to see all available commands");
        }
    }
    return 0;
}
