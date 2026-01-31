#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "debugger/breakpoints.h"
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/user.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include "debugger/utils.h"

static int add_breakpoint(struct debug_frame *dbg_info, int lineno, unsigned char orig_word);
static int add_interrupt(int lineno, unsigned char *bytecode);
static int reinsert_breakpoint(struct debug_frame *dbg_info, int lineno, unsigned char *bytecode);

void print_bytecode(const unsigned char *bytecode, size_t size) {
    if (!bytecode) {
        return;
    }

    for (size_t i = 0; i < size; i++) {
        printf("%02X ", bytecode[i]);
    }
    printf("\n");
}

// This fucntion initializes all the set breakpoint
// Can be used if the user wishes to rerun the program.
void init_breakpoints(struct debug_frame *dbg_info, unsigned char *bytecode) {
    for (int i = 0; i < dbg_info->breakpoint_count; i++){
        dbg_info->breakpoints[i].enabled = 1;
        set_breakpoint(dbg_info, dbg_info->breakpoints[i].lineno, bytecode, 1);
    }
}

// Used to list all the breakpoints
void breakpoint_info(struct debug_frame *dbg_info) {
    if (dbg_info->breakpoint_count == 0){
        print_info("No breakpoints set\n");
        return;
    }

    for(int i = 0; i < dbg_info->breakpoint_count; i++){
        printf("Break point number %d\n", i + 1);
        printf("\tBreakpoint Address : %d\n", dbg_info->breakpoints[i].lineno);
        printf("\tBreakpoint Original Data : 0x%02X\n", dbg_info->breakpoints[i].orig_data);
        char *enabled = dbg_info->breakpoints[i].enabled ? "Yes" : "No";
        printf("\tBreakpoint enabeled %s\n", enabled);
    }
}

//  Used to find the breakpoint's index
int find_breakpoint(struct debug_frame *dbg_info, int lineno) {
    /*
    Input : address to search in breakpoint
    Output:
    breakpoint's index : if breakpoint is present
    -1                 : if breakpoint is not present
    */
    for (int i = 0; i < dbg_info->breakpoint_count; i++) {
        if (dbg_info->breakpoints[i].lineno == lineno)
            return i;    }
    return -1;
}

// Add breakpoints to the list of breakpoits
// Internal Function
static int add_breakpoint(struct debug_frame *dbg_info, int lineno, unsigned char orig_word){
    if (dbg_info->breakpoint_count >= MAX_BREAKPOINTS) {
        print_info("Max Breakpoints Reached\n");
        return -1;
    }
    if (find_breakpoint(dbg_info, lineno) != -1) {
        printf(COLOR_ORANGE
            "[DEBUGGER / Info]: No Breakpoint set on "
            VAL_BLUE("%d")
            COLOR_RESET "\n",
            lineno);
        return -1;
    }
    dbg_info->breakpoints[dbg_info->breakpoint_count].lineno = lineno;
    dbg_info->breakpoints[dbg_info->breakpoint_count].orig_data = orig_word;
    dbg_info->breakpoints[dbg_info->breakpoint_count].enabled = true;

    dbg_info->breakpoint_count = dbg_info->breakpoint_count + 1;
    // printf("add_breakpoint returning %d\n", dbg_info->breakpoint_count - 1);
    return dbg_info->breakpoint_count - 1;
}


// Used to disable the breakpoint
// Does not remove the breakpoint from the breakpoint list
int disable_breakpoints(struct debug_frame *dbg_info, int lineno, unsigned char *bytecode) {
    /*
    Inputs : 
    addr -> breakpoint address
    pid -> child pid

    Outputs : 
    0  : Success
    1  : Debugger error
    -1 : Kernel Error
    */

    int breakpoint_index = find_breakpoint(dbg_info, lineno);
    if(breakpoint_index == -1){
        printf(COLOR_ORANGE
            "[Debuuger / Error] : No Breakpoints at "
            VAL_BLUE("%d")
            COLOR_RESET "\n",
            lineno);
        return 1;
    }
    if(dbg_info->breakpoints[breakpoint_index].enabled == false){
        print_error("Breakpoint already Disabled\n");
        return 1;
    }

    bytecode[lineno] = dbg_info->breakpoints[breakpoint_index].orig_data;
    

    dbg_info->breakpoints[breakpoint_index].enabled = false;
    return 0;
}

// Used to enable the breakpoints
int enable_breakpoints(struct debug_frame *dbg_info, int lineno, unsigned char *bytecode){
    int breakpoint_index = find_breakpoint(dbg_info, lineno);
    if(breakpoint_index == -1){
        printf(COLOR_ORANGE
            "[Debuuger / Error] : No Breakpoints at "
            VAL_BLUE("%d")
            COLOR_RESET "\n",
            lineno);
        return 1;
    }
    if(dbg_info->breakpoints[breakpoint_index].enabled == true){
        print_error("Breakpoint already Enabeled\n");
        return 1;
    }
    if (add_interrupt(lineno, bytecode) != 0) return -1;
    dbg_info->breakpoints[breakpoint_index].enabled = 1;
    return 0;
}

// Used to add interuppt(0xCC) at the given address
// internal function
static int add_interrupt(int lineno, unsigned char *bytecode){
    /*
    Inputs : 
    addr -> where the interrpt is supposed to be entered
    pid -> the PID of the child

    Output : 
        0  : Success
        -1 : Kernel Error
    */
    // Set the error number to 0 as it may not have been cleared
    bytecode[lineno] = 0XCC;
    return 0;
}

// Used to set a new breakpoint
int set_breakpoint(struct debug_frame *dbg_info, int lineno, unsigned char *bytecode, int reset) {
    /*
    Inputs :
    addr -> the location where breakpoint is to be set
    pid -> The PID of child

    Outputs : 
    0  : Sucess
    1  : Debugger Error
    -1 : Kernel Error
    */

    if (find_breakpoint(dbg_info, lineno) != -1 && reset != 1){
        print_info("Breakpoint Already set\n");
        return 1;
    }

    // check if the lineno is out of index or not.
    // Not other way around 
    // Need to get the original word in order to enter it into the 
    // database
    unsigned char word = bytecode[lineno];

    int success = add_interrupt(lineno, bytecode);
    if (success != 0) return success;

    if(reset!= 1){
        if(add_breakpoint(dbg_info, lineno, word) < 0){
            // Failed to add breakpoint
            // restore the original value at the address.
            bytecode[lineno] = word;
            perror("set_breakpoint: restore failed");
            return -1;
        }    
        
    }

    return 0;
}

// Used after hitting the breakpoint
static int reinsert_breakpoint(struct debug_frame *dbg_info, int lineno, unsigned char *bytecode) {
    /*
    Inputs : 
    addr -> where the breakpoint is supposed to be reinserted
    pid -> The PID of the child;

    Outputs :
    0  : Success
    1  : Debugger Error
    -1 : Kernel Error
    */

    int breakpoint_index = find_breakpoint(dbg_info, lineno);
    if (breakpoint_index == -1) {
        print_error("No breakpoint\n");
        return 1;
    }
    if (dbg_info->breakpoints[breakpoint_index].enabled == false) {
        print_info("Breakpoint Disabled\n");
        return 1;
    }

    return add_interrupt(lineno, bytecode);
}

int remove_breakpoint(struct debug_frame *dbg_info, int lineno, unsigned char *bytecode) {
    /*
    Inputs : 
    addr -> where the breakpoint is supposed to be removed from
    pid -> The PID of the child;

    Outputs :
    0  : Success
    1  : Debugger Error
    -1 : Kernel Error
    */

    int breakpoint_index = find_breakpoint(dbg_info, lineno);

    // if no breakpoint is present on the address
    if (breakpoint_index == -1) {
        printf(COLOR_RED
            "[Debuuger / Error] : No Breakpoints at "
            VAL_BLUE("%d")
            COLOR_RESET "\n",
            lineno);
        return 1;
    }

    // if breakpoint is enabeled then we need to get
    // the original data back
    if (dbg_info->breakpoints[breakpoint_index].enabled) {
        bytecode[dbg_info->breakpoints[breakpoint_index].lineno] = dbg_info->breakpoints[breakpoint_index].orig_data;
    }

    // shift back the succesiding breakpoints
    for (int i = breakpoint_index; i < dbg_info->breakpoint_count - 1; i++) {
        dbg_info->breakpoints[i] = dbg_info->breakpoints[i + 1];
    }

    dbg_info->breakpoint_count--;

    return 0;
}
// Used for handling the breakpoints
// When they are hit.
int handle_breakpoint(struct process *proc, struct debug_frame *dbg_info){

    int breakpoint_addr = proc->vm->pc;
    int breakpoint_index = find_breakpoint(dbg_info, breakpoint_addr);
    if (breakpoint_index == -1) {
        /* not a user-set breakpoint (could be single-step or exec), handle elsewhere */
        return 1;
    }

    /* 1) restore original instruction word */
    unsigned char orig = dbg_info->breakpoints[breakpoint_index].orig_data;
    proc->vm->code[breakpoint_addr] = orig;
    
    //need to do a single step
    return 0;
    // curr_bpaddr = breakpoints[breakpoint_index].addr;
    
}