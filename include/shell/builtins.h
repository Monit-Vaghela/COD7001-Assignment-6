#ifndef BUILTINS_H
#define BUILTINS_H

#include "shell/tokenizer.h" /* ensures struct token_list is known here */
#include "shell/parser.h"
#include "shell/history.h"
#include "shell/utils.h"
#include "shell/processManager.h"

int handle_builtin(struct base_command *cmd);

#endif /* BUILTINS_H */