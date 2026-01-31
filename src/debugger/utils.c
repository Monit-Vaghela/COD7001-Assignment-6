#include <stdio.h>
#include <stdlib.h>
#include "debugger/utils.h"

void die(const char *msg) {
    perror(msg);
    exit(1);
}


void print_error(const char *msg)
{
    printf(COLOR_RED "[DEBUGGER / Error] : %s" COLOR_RESET "\n", msg);
}

void print_info(const char *msg)
{
    printf(COLOR_ORANGE "[DEBUGGER / Info] : %s" COLOR_RESET "\n", msg);
}

void print_debugger(const char *msg)
{
    printf(COLOR_BLUE "[DEBUGGER] : %s" COLOR_RESET "\n", msg);
}