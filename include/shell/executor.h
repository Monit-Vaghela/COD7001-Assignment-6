#ifndef EXECUTOR_H
#define EXECUTOR_H
#include "shell/tokenizer.h"
#include "shell/parser.h"

// FUNCTIONS
void reap_zombies();
int run_pipeline_command(struct command *cmd);
int run_base_command(struct base_command *bc);
int execute_command(struct command *cmd);
#endif