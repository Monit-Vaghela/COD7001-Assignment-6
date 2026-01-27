#ifndef PARSER_H
#define PARSER_H

#include "shell/tokenizer.h"

#define MAX_ARGS 128
// The Parser works on Linked list structure.
// It traverses through the the tokens and whenever it see
// | operator it creates a base command.
// each base command holds the arguments and all the other 
// Impotant flags and variables required to run that argument.
/*
ex : 
1) minishell > ls
- this will create base_command of frame
    BS -> NULL
2) minishell > cmd1 | cmd2
- this will create a link list of length 2
    HEAD -> BS -> NULL
- Later in executor we will implment pipeline by traversing
the link list.
*/
struct base_command {
    char *argv[MAX_ARGS];
    int argc;
    char *in_file;
    char *out_file;
    int append;
    int background;
    struct base_command *next;
};

struct command {
    int count;
    struct base_command *head;
};

// Functions.
void free_command(struct command *cmd);
void init_base_cmd(struct base_command *bc);
int parse_base_command(struct token_list *tokens, struct base_command *cmd, int start, int end);
int parse_command(struct token_list *tokens, struct command *cmd);
void print_command(const struct command *cmd);

#endif