#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H
#include "parser/ast.h"
#include "ir/ir.h"
#include <stdlib.h>
#include <string.h>
#include "VM/vm.h"

// Linked list used to store the processes.
enum proc_state {
    PROC_EMPTY,
    PROC_SUBMITTED,
    PROC_RUNNING,
    PROC_DEBUG,
    PROC_TERMINATED
};
typedef struct source_mapping {
    char *instruction;
    int lineno;
    int instruction_start;
}source_mapping;

struct process {
    int pid;
    enum proc_state state;

    char program_path[256];

    struct ast_node *ast;
    source_mapping *map;
    int map_count;
    int max_pc;
    int lines;
    char *ir;

    vm *vm;

    int exit_code;
};

struct proc_node {
    struct process *proc;
    struct proc_node *next;
};


int proc_list_add(const struct process *p);
struct process *proc_list_find(int pid);
int proc_list_remove(int pid);
void proc_list_free();
struct process *process_create(const char *program_path);
void print_proc_list();
void print_source_mappings(struct process *proc);
#endif