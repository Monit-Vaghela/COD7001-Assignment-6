#include <stdio.h>
#include <string.h>
#include <signal.h>
#include "shell/parser.h"
#include "shell/executor.h"
#include "shell/utils.h"
#include "shell/history.h"
#include "shell/shell.h"
#include "shell/processManager.h"


#define COLOR_GREEN   "\033[32m" // colour green
#define COLOR_BLUE    "\033[34m" // colour blue
#define COLOR_RESET   "\033[0m" // colour reset

#define PATH_MAX 128 // max command length.

// Global variables
char history[HISTORY_SIZE][MAX_LEN];
int history_count = 0;
int history_head = 0;
int bg_processes = 0;

void start_shell() {
    print_proc_list();
    history_count = load_history(history);
    if (history_count < 0) {
        printf("failed to load history\n");
    }

    history_head = history_count % HISTORY_SIZE;
    // print_history(history, history_head, history_count);
    // Ignores SIGINT (signal sent when CTRL + C is pressed)
    // This will just tell the shell that when this signal arrives
    // we will ignore it.
    signal(SIGINT, SIG_IGN);
    char line[1024];
    char cwd[PATH_MAX];
    struct token_list toks;
    struct command cmd;
    while (1) {
        get_current_directory(cwd, sizeof(cwd));
        printf(COLOR_GREEN "mini-shell>: " COLOR_BLUE "%s" COLOR_RESET "$ " , cwd);
        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("\n");
            save_history(history, history_head, history_count);
            break;
        }
        // strip trailing newline if present
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }
        if (line[0] == '\n') continue;

        // printf("ADDING HISTORY\n");
        history_count = add_history(history, &history_head, history_count, line);
        // print_history(history, history_head, history_count);

        if (tokenize_line(line, &toks) != 0) continue;
        if (parse_command(&toks, &cmd) != 0) {
            free_tokens(&toks);
            continue;
        }
        execute_command(&cmd);
        free_tokens(&toks);
        free_command(&cmd);
        reap_zombies();
    }
}
