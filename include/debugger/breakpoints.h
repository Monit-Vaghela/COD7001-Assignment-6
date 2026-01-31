#ifndef BREAKPOINTS_H
#define BREAKPOINTS_H
#include "debugger/utils.h"
#include <sys/types.h>
#include "shell/processManager.h"

void init_breakpoints(struct debug_frame *dbg_info, unsigned char *bytecode);
void breakpoint_info(struct debug_frame *dbg_info);
int find_breakpoint(struct debug_frame *dbg_info, int lineno);
int disable_breakpoints(struct debug_frame *dbg_info, int lineno, unsigned char *bytecode);
int enable_breakpoints(struct debug_frame *dbg_info, int lineno, unsigned char *bytecode);
int set_breakpoint(struct debug_frame *dbg_info, int lineno, unsigned char *bytecode, int reset);
int handle_breakpoint(struct process *proc, struct debug_frame *dbg_info);
int remove_breakpoint(struct debug_frame *dbg_info, int lineno, unsigned char *bytecode);
void print_bytecode(const unsigned char *bytecode, size_t size) ;

#endif