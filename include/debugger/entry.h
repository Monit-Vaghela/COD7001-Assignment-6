#ifndef ENTRY_H
#define ENTRY_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <unistd.h>
#include <string.h>
#include "debugger/debugger.h"
#include <signal.h>
#include "shell/processManager.h"
#include "debugger/breakpoints.h"

int debug(struct process *proc);
#endif /* MEMORY_H */