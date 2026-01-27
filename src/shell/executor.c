#include <stdio.h>
#include "shell/executor.h"
#include "shell/builtins.h"
#include "shell/parser.h"
#include <unistd.h>     // fork(), execvp(), _exit()
#include <sys/wait.h>   // waitpid()
#include <sys/types.h>  // pid_t
#include <fcntl.h>
#include <signal.h>


/* There are 2 type of redirection
1) Input redirection
Example
cat < input.txt
- cat normally reads from STDIN 0 and writes to standard output 1
- < input.txt means replace the contents of STDIN with input.txt
basically push everything from input.txt to STDOUT

2) Output redirection
ls > output.txt
- ls gives a list of files in current directory
- > output.txt, with this we write that ouput to this file
*/

int redirection_command(struct base_command *cmd){
    /*
    Input : struct base_command
    Output : 
        -1 : Kernel Error
        0  : Successful
    */
    if(cmd->in_file){
        // We can use O_RDWR; if our code is not writing anywhere, its
        // perfectly safe.
        int fd = open(cmd->in_file, O_RDONLY);

        // Couldn't open input file.
        if(fd < 0){
            perror("Open Input");
            return -1;
        }
        // Couldn't update the FD.
        if(dup2(fd, STDIN_FILENO) < 0){
            perror("Dup2 input");
            close(fd);
            return -1;
        }
        close(fd);
    }

    if(cmd->out_file){
        // We can use O_RDWR, its perfectly safe.
        // now there are 2 types of output redirection
        // 1) output redirection, if file exists then we
        //      we overwrite on it
        // 2 append output redirection, if file exists then
        //      we append to it
        int fd;
        if (cmd->append){
            fd = open(cmd->out_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
        }else{
            fd = open(cmd->out_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        }

        // Couldn't open input file.
        if(fd < 0){
            perror("Open output");
            return -1;
        }
        // Couldn't update the FD.
        if(dup2(fd, STDOUT_FILENO) < 0){
            perror("Dup2 output");
            close(fd);
            return -1;
        }
        close(fd);
    }
    return 0;
}

//Zombie Process cleanup
void reap_zombies(void) {
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0) {
        dcr_bg_processes();
    }
}

int run_base_command(struct base_command *cmd){
    /*
    Input : struct base_command
    Output : 
        -1 : Kernel Error
        0  : Successful
    */

   // if command is empty just pass no need to run anything.
    if (!cmd || cmd->argc == 0) return 0;

    // first try builtin commands.
    if (handle_builtin(cmd) == 0) return 0;

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {
        // We set to CTRL + C behaviour to normal for child process
        // So child can be killed with CTRL + C
        signal(SIGINT, SIG_DFL);
        if (redirection_command(cmd) < 0)
            _exit(1);
        cmd->argv[cmd->argc] = NULL;
        execvp(cmd->argv[0], cmd->argv);
        perror("execvp");
        _exit(1);

    } else {
        if (!cmd->background) {
            int status;
            if (waitpid(pid, &status, 0) == -1) {
                perror("waitpid");
                return -1;
            }
        } else {
            inc_bg_processes();
            printf("[bg] %d\n", pid);
            return 0;
        }
    }
    return 0;
}

/*
For pipeline We are only allowing the last command to have background 
process, if any process in between has & symbol we reject
right now we are only executing a single pipeline. will later update to
implement multiple pipeline.
Technically this function should use run_base_command. but that will come
Later.
*/
int run_pipeline_command(struct command *cmd){
    /*
    Input : struct command
    Output : 
        -1 : Kernel Error
        0  : Successful
        1  : Executor Error
    */
    struct base_command *first  = cmd->head;
    struct base_command *second = first->next;

    if (first->background) {
        printf("Executor error: only the last command in pipeline can be background\n");
        return 1;
    }
    // Basic logic for a single pipeline for now.
    /* Example
    A | B
    */
    int pipefd[2];
    if (pipe(pipefd) < 0) {
        perror("pipe");
        return -1;
    }

    // Because we have used fork, each child will get a copy of pipefd
    // !!! SO REMEMBER TO CLOSE IN BOTH THE CHILDS!!!!!
    pid_t p1 = fork();
    if (p1 < 0) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }

    if (p1 == 0) {
        // We set to CTRL + C behaviour to normal for child process
        // So child can be killed with CTRL + C
        signal(SIGINT, SIG_DFL);

        // First command: stdout -> pipe write end
        if (dup2(pipefd[1], STDOUT_FILENO) < 0) {
            perror("dup2");
            _exit(1);
        }

        // ???
        close(pipefd[0]);
        close(pipefd[1]);

        // No builtins here for now; always exec external
        if (redirection_command(first) < 0)
            _exit(1);

        first->argv[first->argc] = NULL;
        execvp(first->argv[0], first->argv);
        perror("execvp");
        _exit(1);
    }

    pid_t p2 = fork();
    if (p2 < 0) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }

    if (p2 == 0) {
        // We set to CTRL + C behaviour to normal for child process
        // So child can be killed with CTRL + C
        signal(SIGINT, SIG_DFL);
        
        // Second command: stdin <- pipe read end
        if (dup2(pipefd[0], STDIN_FILENO) < 0) {
            perror("dup2");
            _exit(1);
        }
        close(pipefd[0]);
        close(pipefd[1]);

        if (redirection_command(second) < 0)
            _exit(1);

        second->argv[second->argc] = NULL;
        execvp(second->argv[0], second->argv);
        perror("execvp");
        _exit(1);
    }

    close(pipefd[0]);
    close(pipefd[1]);
    
    if (!second->background) {
        int status;
        waitpid(p1, &status, 0);
        waitpid(p2, &status, 0);
    } else {
        printf("[bg] %d %d\n", p1, p2);
    }

    return 0;
}

// For executing commands we will do it in 2 ways
// 1) If the command has pipeline then we will only execute left base command
//    then pipeline its output to right base command.
// 2) Else only need to execute left base command, Now left base command,
//    will either it is builtin command or a external command
int execute_command(struct command *cmd) {
    /*
    Input : struct command
    Output : 
        -1 : Kernel Error
        0  : Successful
        1  : Executor Error
    */

    // If head comes back empty, technically cant happen as we are
    // taking care of that in previous layer.
    if (!cmd || !cmd->head) return 0;

    //Single command (no Pipeline)
    if(cmd->count == 1){
       return run_base_command(cmd->head);
    }else{
        return run_pipeline_command(cmd);
    }
    
}