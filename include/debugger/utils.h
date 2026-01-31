#ifndef UTILS_H
#define UTILS_H

#define MAX_BREAKPOINTS 128
typedef enum {
    LOADED,
    RUNNING,
    STOPPED,
    EXITED
} dbg_state;

typedef struct {
    int lineno;
    unsigned char orig_data;
    int enabled;
} breakpoint_t;

struct debug_info{
    dbg_state state; // Initital State
    breakpoint_t breakpoints[MAX_BREAKPOINTS];
    int breakpoint_count;
};
#define COLOR_RESET   "\033[0m"

#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_ORANGE  "\033[33m"
#define VAL_BLUE(x) COLOR_BLUE x COLOR_ORANGE

void print_error(const char *msg);
void print_info(const char *msg);
void print_debugger(const char *msg);


void die(const char *msg);

#endif
