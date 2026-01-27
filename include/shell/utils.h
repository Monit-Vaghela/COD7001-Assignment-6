#ifndef UTILS_H
#define UTILS_H

#include "shell/history.h"
extern int bg_processes;

void dcr_bg_processes();
void inc_bg_processes();
void print_bg_processes();

void get_current_directory(char *cwd, size_t size);
void print_history(char history[][MAX_LEN], int head, int count);
int add_history(char history[][MAX_LEN], int *head, int count, const char *line);
int save_history(char history[][MAX_LEN], int head, int count);
int load_history(char history[][MAX_LEN]);

#endif