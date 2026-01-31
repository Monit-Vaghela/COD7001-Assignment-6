#ifndef WATCHPOINTS_H
#define WATCHPOINTS_H

#include <sys/types.h>

int add_watchpoint(unsigned long addr, pid_t pid);
void check_watchpoints(pid_t pid);
void watchpoint_info(void);
void init_watchpoints(void);
int disable_watchpoints(unsigned long addr);
int enable_watchpoints(unsigned long addr);

#endif
