#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "shell/parser.h"
#include "shell/tokenizer.h"

// This is just for debugging, no actual use
void print_command(const struct command *cmd) {
    const struct base_command *bc = cmd->head;
    int idx = 0;
    while (bc) {
        printf("Command %d:\n", idx);
        printf("  argc = %d\n", bc->argc);
        for (int i = 0; i < bc->argc; i++) {
            printf("    argv[%d] = '%s'\n", i, bc->argv[i]);
        }
        printf("  in_file  = %s\n", bc->in_file  ? bc->in_file  : "(none)");
        printf("  out_file = %s\n", bc->out_file ? bc->out_file : "(none)");
        printf("  append   = %d\n", bc->append);
        printf("  background = %d\n", bc->background);
        bc = bc->next;
        idx++;
    }
}

// As we are using malloc if we dont free the allocated space 
// Then continous running of commmand may result in memory leak
// This function helps us from that.
void free_command(struct command *cmd) {
    struct base_command *cur = cmd->head;
    while (cur) {
        struct base_command *next = cur->next;
        free(cur);
        cur = next;
    }
    cmd->head = NULL;
}


// Function to initialize the base command with 0 and null values
void init_base_cmd(struct base_command *bc){
    bc->argc = 0;
    bc->in_file = NULL;
    bc->out_file = NULL;
    bc->append = 0;
    bc->background = 0;
    bc->next = NULL;
    // Set each of the arguments to NULL initially so we dont have
    // to specially change the last character to null later.      
    for (int i = 0; i < MAX_ARGS; i++) {
        bc->argv[i] = NULL;
    }
}

/*
This function will parse the base commands inside the pipe line
I have divided this into 3 parts
1) Arguments : Actual arguments, e.g : ls, pwd etc etc
2) I/O redirection : parsing '<' and '>' any thing left to it will be arguments,
                     and anything right to it will be files
3) Background process : the last token of any base command if is '&' then it is a 
                        background process
*/
int parse_base_command(struct token_list *tokens, struct base_command *cmd, int start, int end){
    /*
    Inputs : struct token_list *tokens, struct base_command *cmd, int start, int end
    Outputs :
        0  : Successful
        1  : Parser Error
    */
    int i = start;
    while(i < end){
        if (strcmp(tokens->tokens[i], "<") == 0) {
            if (i + 1 >= end) {
                printf("Parser Error: expected filename after <\n");
                return 1;
            }
            if (cmd->in_file != NULL) {
                printf("Parser Error: multiple input redirections\n");
                return 1;
            }
            cmd->in_file = tokens->tokens[i + 1];
            i += 2;
            continue;
        }
        if (strcmp(tokens->tokens[i], ">") == 0 || strcmp(tokens->tokens[i], ">>") == 0) {
            int append = (tokens->tokens[i][1] == '>');
            if (i + 1 >= end) {
                printf("Parser Error: expected filename after > or >>\n");
                return 1;
            }
            if (cmd->out_file != NULL) {
                printf("Parser Error: multiple output redirections\n");
                return 1;
            }
            cmd->out_file = tokens->tokens[i + 1];
            cmd->append = append;
            i += 2;
            continue;
        }
        if (strcmp(tokens->tokens[i], "&") == 0) {
            cmd->background = 1;
            if (i != end - 1) {
                if(strcmp(tokens->tokens[i + 1], "&") == 0){
                    printf("Parser Error : && not implemented yet\n");
                    return 1;
                }
                printf("Parser Error: & must be last\n");
                return 1;
            }
            i++;
            continue;
        }
        // printf("THIS ARGUMENT IS NON OPERATOR");
        if(cmd-> argc >= MAX_ARGS - 1){
            printf("Parser Error : too many arguments\n");
            return 1;
        }else{
            // printf("Adding non opertor arguments");
            cmd->argv[cmd->argc++] = tokens->tokens[i];
            cmd->argv[cmd->argc] = NULL;
        }

        i++;
    }
    return 0;
    
}

// This function takes the tokens iterates through them and creates a linked
// list of base commands.
int parse_command(struct token_list *tokens, struct command *cmd) {
    cmd->head = NULL;
    cmd->count = 0;

    struct base_command *current = NULL;
    int command_start = 0;
    int n = 0;
    if (tokens->count == 0) return 1;

    for (int i = 0; i <= tokens->count; i++) {
        // If the current token is an | then it must not be the end
        // or if it is the end, then we create a base command
        int is_pipe_or_end = 0;

        if (i == tokens->count) {
            // virtual '|' at end
            is_pipe_or_end = 1;
        } else if (strcmp(tokens->tokens[i], "|") == 0) {
            is_pipe_or_end = 1;
        }

        if (is_pipe_or_end) {
            // We need to explicitly check that there if | is not alone.
            // Cases that we need to take care of 
            // 1) ls |
            // 2) | ls
            // 3) |
            // 4) 
            if (command_start == i) {
                printf("Parser Error: missing command near '|'\n");
                free_command(cmd);
                return 1;
            }

            // Allocate space and handle malloc error.
            struct base_command *node = malloc(sizeof(struct base_command));
            if (!node) {
                perror("malloc");
                free_command(cmd);
                return -1;
            }
            init_base_cmd(node);

            // Now we just parse the tokens from command_start to this index
            // and if it fails we free the nodes
            if (parse_base_command(tokens, node, command_start, i) != 0) {
                free(node);
                free_command(cmd);
                return 1;
            }

            // do link list logic, apply current nodes next to this new nodes
            // address
            node->next = NULL;
            if (!cmd->head) {
                cmd->head = node;
            } else {
                current->next = node;
            }
            current = node;
            n++;
            cmd->count = n;

            //printf("DONE ALLOCATION SPACES\n");
            
            // We will only implement single pipeline
            // but this creates a base for multipiples
            if (n > 2) {
                printf("Parser Error: multiple pipelines not supported\n");
                free_command(cmd);
                return 1;
            }

            command_start = i + 1;
        }   
    }

    // If no base_command was created at all (empty input)
    if (!cmd->head) {
        printf("Parser Error: missing command\n");
        return 1;
    }
    // If successful
    return 0; 
}