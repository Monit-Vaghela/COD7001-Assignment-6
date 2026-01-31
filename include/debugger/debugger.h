#ifndef DEBUGGER_H
#define DEBUGGER_H
#include "shell/processManager.h"
#include "debugger/breakpoints.h"
#include "utils.h"
extern unsigned long curr_bpaddr;

int debugger_run(struct process *proc, struct debug_info *dbg_info);
int debugger_step(struct process *proc, struct debug_info *dbg_info);
int debugger_continue(struct process *proc, struct debug_info *dbg_info);
// int debugger_regs(pid_t child_pid, dbg_state state, int isExtended);
int debugger_quit();
// int debugger_watchpoints(unsigned long addr, pid_t pid, dbg_state state);
// int debugger_memory_inspection(unsigned long addr, pid_t pid, int count, dbg_state state);
const char *debugger_state_to_string(struct debug_info *dbg_info);

#endif
