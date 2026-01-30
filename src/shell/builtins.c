#include <stdlib.h>   /* for getenv */
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>

#include "shell/builtins.h"
#include "shell/tokenizer.h"
#include "shell/compiler.h"
#include "errno.h"

void pwd() {
    char *cwd = getcwd(NULL, 0);
    if (!cwd) {
        perror("getcwd");
        return;
    }
    printf("%s\n", cwd);
    free(cwd);
}

// Only for this function return for wrong is -1
static int check_pid(const char* pid, const char* command_name){
    char *endptr;
    errno = 0;

    int id = (int)strtol(pid, &endptr, 10);

    /* Check for conversion errors */
    if (errno != 0 || *endptr != '\0') {
        printf("Error: ID must be a valid integer\n");
        return -1;
    }

    if (id == 0){
        printf("Access Denied: user cannot %s shell\n", command_name);
        return -1;
    }
    if(id < 0){
        printf("PID Cannot be negative\n");
        return -1;
    }
    return id;
}

static int execute_process(struct base_command *cmd, bool debug){
    if(cmd->argc > 2) {
        printf("Usage: %s <id>\n", cmd->argv[0]);
        return 0;
    }

    int id = check_pid(cmd->argv[1], "run");
    if(id == -1) return 0;

    //first we find the PID
    struct process *proc = proc_list_find(id);
    if(proc == NULL){
        printf("No Process with pid %d\n", id);
        return 0;
    }
    //change the state to running
    proc->state = PROC_RUNNING;
    //Then we first compile the process
    int Compiled = compile(proc);
    proc->state = PROC_TERMINATED;
    return Compiled;
}

static int kill_process(struct base_command *cmd){
    if(cmd->argc > 2) {
        printf("Usage: %s <id>\n", cmd->argv[0]);
        return 0;
    }

    int id = check_pid(cmd->argv[1], "kill");
    if(id == -1) return 0;

    //1) find the process
    struct process *proc = proc_list_find(id);
    if(proc == NULL){
        printf("No Process with pid %d\n", id);
        return 0;
    }

    //2) Whatever its state may be, clear all the resources allocated
    // and the remove from the linked list.
    return 0;
}

static int debug_memstat(struct base_command *cmd){
    if(cmd->argc > 2) {
        printf("Usage: %s <id>\n", cmd->argv[0]);
        return 0;
    }
    int id = check_pid(cmd->argv[1], "kill");
    if(id == -1) return 0;

    //1) find the pid
    struct process *proc = proc_list_find(id);
    if(proc == NULL){
        printf("No Process with pid %d\n", id);
        return 0;
    }

    //2) check its state, if submitted then cant perform memstate
    if(proc->state == PROC_SUBMITTED){
        printf("process %d is yet to be runned, can't perform memstat\n", proc->pid);
        return 0;
    }

    //3) now we check if the vm heap head is not null and then print 
    return 0;
}

static int garbage_collection(struct base_command *cmd){
    if(cmd->argc > 2) {
        printf("Usage: %s <id>\n", cmd->argv[0]);
        return 0;
    }
    int id = check_pid(cmd->argv[1], "gc");
    if(id == -1) return 0;

    //1) find the pid
    struct process *proc = proc_list_find(id);
    if(proc == NULL){
        printf("No Process with pid %d\n", id);
        return 0;
    }

    // check the state, if submitted cant perform gc
    if(proc->state == PROC_SUBMITTED){
        printf("process %d is yet to be runned, can't perform gc\n", proc->pid);
        return 0;
    }

    // perform garbage collection
    return 0;
}

static int leaks_check(struct base_command *cmd){
    // to perform leak check, perform mark algo only, and
    // print out the unreachable objects
    return 0;
}

// This function handles only 3 shell commands
// 1) exit -> exit from the shell
// 2) cd -> classic change directory
//   2.1) cd : move to home directory
//   2.2) 
int handle_builtin(struct base_command *cmd) {
    // print_history(history, history_head, history_count);
    if (cmd->argc == 0) return 1;
    if (strcmp(cmd->argv[0], "submit") == 0) {
        // printf("next input is : %s\n", cmd->argv[1]);
        if(cmd->argc > 2){
            printf("Usage %s <filename>\n", cmd->argv[0]);
            return 1;
        }

        // Check if the file exists or not
        if (access(cmd->argv[1], F_OK) != 0) {
            printf("No Such file or directory\n");
            return 0;
        }

        // check the extension of the file
        const char *dot = strrchr(cmd->argv[1], '.');
        if (!dot || dot == cmd->argv[1])
            return 0;
        if(strcmp(dot, ".am") != 0){
            printf("Submit only accepts .am extenstions\n");
            return 0;
        }

        struct process *p = process_create(cmd->argv[1]);
        if(proc_list_add(p) != 0){
            printf("Unable to create process\n");
        }
        return 0;
    }

    if (strcmp(cmd->argv[0], "proc_info") == 0) {
        print_proc_list();
        return 0;
    }

    if (strcmp(cmd->argv[0], "run") == 0) {
        return execute_process(cmd, false);
    }

    if(strcmp(cmd->argv[0], "debug") == 0) {
        return execute_process(cmd, true);
    }

    if(strcmp(cmd->argv[0], "kill") == 0){
        return kill_process(cmd);
    }

    if(strcmp(cmd->argv[0], "memstat") == 0){
         return debug_memstat(cmd);
    }

    if(strcmp(cmd->argv[0], "gc") == 0){
        return garbage_collection(cmd);
    }

    if(strcmp(cmd->argv[0], "leaks") == 0){
        return leaks_check(cmd);
    }

    if (strcmp(cmd->argv[0], "exit") == 0) {
        // print_history(history, history_head, history_count);

        // clear the processes
        proc_list_free();
        save_history(history, history_head, history_count);
        _exit(0);
    }

    if (strcmp(cmd->argv[0], "cd") == 0) {
        char *t_dir;
        // If cd is alone than we need to go to home
        // DIR
        if(cmd->argc == 1){
            t_dir = getenv("HOME");
        }else{
            t_dir = cmd->argv[1];
        }
        if (strcmp(t_dir, "~") == 0) t_dir = getenv("HOME");
        if (chdir(t_dir) == -1) perror("cd");
        return 0;
    }

    if (strcmp(cmd->argv[0], "pwd") == 0) {
        pwd();
        return 0;
    }

    if (strcmp(cmd->argv[0], "history") == 0) {
        print_history(history, history_head, history_count);
        return 0;
    }

    if (strcmp(cmd->argv[0], "bg") == 0) {
        print_bg_processes();
        return 0;
    }

    return 1;
}