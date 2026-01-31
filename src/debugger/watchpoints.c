#include "debugger/watchpoints.h"
#include "debugger/utils.h"
#include <sys/ptrace.h>
#include <errno.h>
#include <stdio.h>

#define MAX_WATCHPOINTS 128

typedef struct {
    unsigned long addr;
    long last_value;
    int enabled;
} watchpoint_t;

static watchpoint_t watchpoints[MAX_WATCHPOINTS];
static int watchpoint_count = 0;

static int find_watchpoint(unsigned long addr)
{
    for (int i = 0; i < watchpoint_count; i++) {
        if (watchpoints[i].addr == addr) {
            return i;   // we found the watchpoint 
        }
    }
    return -1; // no watchpoint found in records
}

int add_watchpoint(unsigned long addr, pid_t pid)
{
    if (watchpoint_count >= MAX_WATCHPOINTS) {
        print_error("Maximum watchpoints reached");
        return 1;
    }

    if (find_watchpoint(addr) != -1) {
        printf(COLOR_ORANGE
            "[Debuuger / Info] : Watchpoint already exists at "
            VAL_BLUE("0x%lx")
            COLOR_RESET "\n",
            addr);
        return 1;
    }

    errno = 0;
    long value = ptrace(PTRACE_PEEKDATA, pid, (void *)addr, NULL);
    if (value == -1 && errno != 0) {
        perror("PTRACE_PEEKDATA");
        return -1;
    }

    watchpoints[watchpoint_count].addr = addr;
    watchpoints[watchpoint_count].last_value = value;
    watchpoints[watchpoint_count].enabled = 1;
    watchpoint_count++;

    printf(COLOR_ORANGE
            "[Debuuger / Info] : Watchpoint set at "
            VAL_BLUE("0x%lx")
            COLOR_RESET "\n",
            addr);
    return 0;
}

int disable_watchpoints(unsigned long addr)
{
    int idx = find_watchpoint(addr);
    if (idx == -1) {
        print_error("No watchpoint at given address");
        return 1;
    }

    if (!watchpoints[idx].enabled) {
        print_error("Watchpoint already disabled");
        return 1;
    }

    watchpoints[idx].enabled = 0;
    return 0;
}

int enable_watchpoints(unsigned long addr)
{
    int idx = find_watchpoint(addr);
    if (idx == -1) {
        print_error("No watchpoint at given address");
        return 1;
    }

    if (watchpoints[idx].enabled) {
        print_error("Watchpoint already enabled");
        return 1;
    }

    watchpoints[idx].enabled = 1;
    return 0;
}

void check_watchpoints(pid_t pid)
{
    for (int i = 0; i < watchpoint_count; i++) {

        if (!watchpoints[i].enabled)
            continue;

        errno = 0;
        long current_value =ptrace(PTRACE_PEEKDATA, pid, (void *)watchpoints[i].addr, NULL);

        if (current_value == -1 && errno != 0) {
            perror("PTRACE_PEEKDATA (watchpoint)");
            continue;
        }

        if (current_value != watchpoints[i].last_value) {

            printf("[WATCHPOINT] Memory changed at 0x%lx\n", watchpoints[i].addr);
            printf("Old value: 0x%lx\n", watchpoints[i].last_value);
            printf("New value: 0x%lx\n", current_value);

            watchpoints[i].last_value = current_value;
        }
    }
}

void init_watchpoints(void)
{
    watchpoint_count = 0;
}

void watchpoint_info(void)
{
    if (watchpoint_count == 0) {
        print_info("No watchpoints set");
        return;
    }

    for (int i = 0; i < watchpoint_count; i++) {

        printf("Watchpoint %d:\n", i);
        printf("    Address     : 0x%lx\n", watchpoints[i].addr);
        printf("    Last value  : 0x%lx\n", watchpoints[i].last_value);
        printf("    Enabled     : %s\n", watchpoints[i].enabled ? "Yes" : "No");
    }
}
