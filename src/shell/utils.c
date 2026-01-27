#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "shell/utils.h"

void dcr_bg_processes(){
    if(bg_processes == 0){
        printf("Utility Error : There are no background processes");
        return;
    }
    bg_processes--;
}

void inc_bg_processes(){
    bg_processes++;
    return;
}

void print_bg_processes(){
    if(bg_processes == 0){
        printf("No Background Processes\n");
        return;
    }
    printf("Currently %d background processes are running\n", bg_processes);
}

void get_current_directory(char *cwd, size_t size){
    if (getcwd(cwd, size) == NULL) {
        strncpy(cwd, "unknown", size - 1);
        cwd[size - 1] = '\0';
    }
}

int add_history(char history[][MAX_LEN], int *head, int count, const char *line) {
    if (line == NULL || line[0] == '\0')
        return count;

    strncpy(history[*head], line, MAX_LEN - 1);
    history[*head][MAX_LEN - 1] = '\0';

    *head = (*head + 1) % HISTORY_SIZE;

    if (count < HISTORY_SIZE)
        count++;

    return count;
}

void print_history(char history[][MAX_LEN], int head, int count) {
    if (count == 0) {
        printf("History is empty\n");
        return;
    }

    int start = (head - count + HISTORY_SIZE) % HISTORY_SIZE;

    for (int i = 0; i < count; i++) {
        int idx = (start + i) % HISTORY_SIZE;
        if (history[idx][0] != '\0')
            printf("%s\n", history[idx]);
    }   
}

int save_history(char history[][MAX_LEN], int head, int count) {
    FILE *fp = fopen(HISTORY_PATH, "w");
    if (!fp) {
        perror("fopen");
        return -1;
    }

    if (count == 0) {
        fclose(fp);
        return 0;
    }

    int start = (head - count + HISTORY_SIZE) % HISTORY_SIZE;

    for (int i = 0; i < count; i++) {
        int idx = (start + i) % HISTORY_SIZE;
        if (history[idx][0] != '\0') {
            fprintf(fp, "%s\n", history[idx]);
        }
    }

    fclose(fp);
    return 0;
}

/* First we will open from our history.txt the last 32 commands
small nuances that need to be taken care of 
1) if file does not exist, create it.
2) if file exists, but is empty then, need to take care of it
*/
int load_history(char history[][MAX_LEN]) {
    FILE *fp;
    int count = 0;

    fp = fopen(HISTORY_PATH, "a+");
    if (!fp) {
        perror("fopen");
        return -1;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        perror("fseek");
        fclose(fp);
        return -1;
    }

    long size = ftell(fp);
    if (size == -1) {
        perror("ftell");
        fclose(fp);
        return -1;
    }

    rewind(fp);

    while (count < HISTORY_SIZE && fgets(history[count], MAX_LEN, fp)) {
        history[count][strcspn(history[count], "\n")] = '\0';
        count++;
    }

    fclose(fp);
    return count;
}