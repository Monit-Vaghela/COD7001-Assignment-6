#ifndef HISTORY_H
#define HISTORY_H

#define HISTORY_SIZE 32
#define MAX_LEN 1024
#define HISTORY_PATH "log/history.txt"
extern char history[HISTORY_SIZE][MAX_LEN];
extern int history_head;
extern int history_count;

#endif