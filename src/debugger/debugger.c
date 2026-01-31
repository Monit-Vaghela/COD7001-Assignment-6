#include <stdio.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include "debugger/debugger.h"
#include "debugger/breakpoints.h"
#include "debugger/watchpoints.h"
#include "debugger/regs.h"
#include "debugger/utils.h"
#include "debugger/memory.h"
#include <unistd.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include "VM/vm.h"

unsigned long curr_bpaddr = 0;

int debugger_run(struct process *proc, struct debug_info *dbg_info)
{
    if(dbg_info->state != LOADED && dbg_info->state != EXITED){
        print_error("Program already running");
    }else{
        dbg_info->state = RUNNING;
        return vm_run(proc->vm, 0, 0, 1);
    }
    return 1;
}

int debugger_step(struct process *proc, struct debug_info *dbg_info)
{
    if(dbg_info->state != STOPPED){
        print_error("No Program Loaded");
    }else{
        dbg_info->state = RUNNING;
        return vm_run(proc->vm, 0, 0, -1);
    }
    return 1;
}

int debugger_continue(struct process *proc, struct debug_info *dbg_info)
{
    if(dbg_info->state != STOPPED){
        print_error("No Program Loaded");
    }
    else{
        dbg_info->state = RUNNING;
        return vm_run(proc->vm, 0, 0, 1);
    }
    return 1;
}

// int debugger_regs(pid_t child_pid, dbg_state state, int isExtended)
// {
//     if(child_pid<=0)
//     {
//         print_error("No child process exists");
//         return 1;
//     }
//     if(state != STOPPED)
//     {
//         print_error("Cannot read registers as program must be stopped");
//         return 1;
//     }
//     print_registers(child_pid, isExtended);
//     return 0;
// }

int debugger_quit()
{
    return 1;
}

// int debugger_watchpoints(unsigned long addr, pid_t pid, dbg_state state)
// {
//     if (pid <= 0) {
//         print_error("No child process");
//         return 1;
//     }
//     if (state != STOPPED) {
//         print_error("Program must be stopped to set watchpoint");
//         return 1;
//     }
//     add_watchpoint(addr, pid);
//     return 0;
// }

// int debugger_memory_inspection(unsigned long addr, pid_t pid, int count, dbg_state state)
// {
//     if (pid <= 0) {
//         print_error("No child process");
//         return 1;
//     }
//     if (state != STOPPED) {
//         print_error("Program must be stopped to set watchpoint");
//         return 1;
//     }
//     inspect_memory(addr, pid, count);
//     return 0;
// }

const char* debugger_state_to_string(struct debug_info *dbg_info)
{
    dbg_state state = dbg_info->state;
    switch(state)
    {
        case LOADED:
            return "LOADED";
        case RUNNING:
            return "RUNNING";
        case STOPPED:
            return "STOPPED";
        case EXITED:
            return "EXITED";
        default:
            return "UNKNOWN";
    }
}
