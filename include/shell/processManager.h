#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <stdlib.h>
#include <string.h>

// Linked list used to store the processes.
enum proc_state {
    PROC_EMPTY,
    PROC_SUBMITTED,
    PROC_RUNNING,
    PROC_DEBUG,
    PROC_TERMINATED
};

struct process {
    int pid;
    enum proc_state state;

    char program_path[256];

    void *ast;
    void *ir;

    void *vm_ctx;
    void *mem_ctx;

    int exit_code;
};

struct proc_node {
    struct process proc;
    struct proc_node *next;
};

int proc_list_add(const struct process *p);
struct process *proc_list_find(int pid);
int proc_list_remove(int pid);
void proc_list_free();
struct process *process_create(const char *program_path);
void print_proc_list();
#endif