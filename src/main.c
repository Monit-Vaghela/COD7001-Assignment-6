#include "shell/shell.h"
#include "shell/processManager.h"
#include "string.h"
#include <stdio.h>

int main(){
    // first we create the initial process shell with PID = 0
    struct process *p = process_create("./myShell");
    p->state = PROC_RUNNING;

    if(proc_list_add(p) != 0){
        printf("Unable to create shell process\n");
        return 0;
    }
    start_shell();
}