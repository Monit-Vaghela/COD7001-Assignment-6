#include <stdio.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include "debugger/debugger.h"
#include "debugger/breakpoints.h"
#include "debugger/utils.h"
#include <unistd.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include "VM/vm.h"

unsigned long curr_bpaddr = 0;

int debugger_run(struct process *proc, struct debug_frame *dbg_info)
{
    if(dbg_info->state != LOADED && dbg_info->state != EXITED){
        print_error("Program already running");
    }else{
        dbg_info->state = RUNNING;
        return vm_run(proc->vm, 0, 0, 1);
    }
    return 1;
}

int debugger_step(struct process *proc, struct debug_frame *dbg_info)
{
    if(dbg_info->state != STOPPED){
        print_error("No Program Loaded");
    }else{
        dbg_info->state = RUNNING;
        return vm_run(proc->vm, 0, 0, -1);
    }
    return 1;
}

int debugger_continue(struct process *proc, struct debug_frame *dbg_info)
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


int debugger_quit()
{
    return 1;
}


const char* debugger_state_to_string(struct debug_frame *dbg_info)
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
