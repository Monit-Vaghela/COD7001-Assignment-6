#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <shell/processManager.h>

static struct proc_node *proc_list = NULL;
static int pidCount = 0;

// For debugging only
static const char *proc_state_to_string(enum proc_state state) {
    switch (state) {
        case PROC_EMPTY:      return "EMPTY";
        case PROC_SUBMITTED:  return "SUBMITTED";
        case PROC_RUNNING:    return "RUNNING";
        case PROC_DEBUG:      return "DEBUG";
        case PROC_TERMINATED: return "TERMINATED";
        default:              return "UNKNOWN";
    }
}

static void free_process_resources(struct process *proc){
    if(proc->ast != NULL){
        ast_free(proc->ast);
    }
    if(proc->vm != NULL){
        free(proc->vm);
    }
    // right now we are just clearing vm but
    // we need to check the heap objects in vm
    // clear them as well and then clear vm.
}

void print_proc_list() {
    struct proc_node *head = proc_list;
    while (head) {
        printf("PID: %d | Program: %s | State: %s\n",
               head->proc.pid,
               head->proc.program_path,
               proc_state_to_string(head->proc.state));

        head = head->next;
    }
}

int proc_list_add(const struct process *p) {
    struct proc_node **head = &proc_list;
    struct proc_node *node = malloc(sizeof(struct proc_node));
    if (!node)
        return -1;

    node->proc = *p;         
    node->next = *head;
    *head = node;

    return 0;
}

struct process *proc_list_find(int pid) {
    struct proc_node *head = proc_list;
    while (head) {
        // printf("HERE\n");
        if (head->proc.pid == pid){
            // printf("returning with pid %d\n", head->proc.pid);
            return &head->proc;
        }
        // printf("Current Pid is %d\n", head->proc.pid);
        head = head->next;
    }
    return NULL;
}

int proc_list_remove(int pid) {
    struct proc_node **head = &proc_list;
    struct proc_node *curr = *head;
    struct proc_node *prev = NULL;

    while (curr) {
        if (curr->proc.pid == pid) {
            if (prev)
                prev->next = curr->next;
            else
                *head = curr->next;
            free_process_resources(curr);
            free(curr);
            return 0;
        }
        prev = curr;
        curr = curr->next;
    }
    return -1;
}

void proc_list_free() {
    struct proc_node **head = &proc_list;
    struct proc_node *curr = *head;
    while (curr) {
        struct proc_node *next = curr->next;
        free(curr);
        curr = next;
    }
    *head = NULL;
}

struct process *process_create(const char *program_path) {
    enum proc_state state = PROC_SUBMITTED;
    struct process *p = malloc(sizeof(struct process));
    if (!p)
        return NULL;

    p->pid = pidCount++;
    p->state = state;

    if (program_path) {
        strncpy(p->program_path, program_path, sizeof(p->program_path) - 1);
        p->program_path[sizeof(p->program_path) - 1] = '\0';
    } else {
        p->program_path[0] = '\0';
    }

    p->ast = NULL;
    p->ir = NULL;
    p->vm = NULL;
    p->mem_ctx = NULL;
    p->exit_code = 0;

    return p;
}