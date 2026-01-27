#include <stdlib.h>   /* for getenv */
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "shell/builtins.h"
#include "shell/tokenizer.h"

void pwd() {
    char *cwd = getcwd(NULL, 0);
    if (!cwd) {
        perror("getcwd");
        return;
    }
    printf("%s\n", cwd);
    free(cwd);
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
        if(cmd->argc > 2) printf("Use submit as: submit <filename.am>\n");
        

        // NEED TO CHECK WHETHER THIS FILES EXITS FROM CURRENT DIR.
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