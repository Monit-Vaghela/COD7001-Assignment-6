#include <stdio.h>
#include <stdint.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include "debugger/regs.h"
#define COLOR_RED   "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_RESET "\033[0m"


static void print_flags(uint64_t rflags);
static struct user_regs_struct prev_regs;
static int has_prev = 0;
static void print_extended_registers(struct user_regs_struct *regs);
static void print_sep(){
    printf("+------+--------------------+------+--------------------+\n");
}
static int reg_changed(unsigned long long curr,
                       unsigned long long prev) {
    return curr != prev;
}
static void print_row_colored(const char *r1, unsigned long long v1,
                              unsigned long long p1,
                              const char *r2, unsigned long long v2,
                              unsigned long long p2) {

    const char *c1 = reg_changed(v1, p1) ? COLOR_GREEN : "";
    const char *c2 = reg_changed(v2, p2) ? COLOR_GREEN : "";

    printf("| %-4s | %s0x%016llx%s | %-4s | %s0x%016llx%s |\n",
           r1, c1, v1, COLOR_RESET,
           r2, c2, v2, COLOR_RESET);
}

static void print_header(const char *title) {
    print_sep();
    printf("| %-53s |\n", title);
    print_sep();
    printf("| REG  | VALUE              | REG  | VALUE              |\n");
    print_sep();
}

// static void print_row(const char *r1, unsigned long long v1,
//                       const char *r2, unsigned long long v2) {
//     printf("| %-4s | 0x%016llx | %-4s | 0x%016llx |\n",
//            r1, v1, r2, v2);
// }

void print_registers(pid_t pid, int extended) {
    if(!has_prev){
        has_prev = 1;
    }
    struct user_regs_struct regs;

    if (ptrace(PTRACE_GETREGS, pid, NULL, &regs) == -1) {
        perror("PTRACE_GETREGS");
        return;
    }

    print_header("GENERAL PURPOSE REGISTERS");

    print_row_colored("RBP", regs.rbp, prev_regs.rbp,"RSP", regs.rsp, prev_regs.rsp);
    print_row_colored("RAX", regs.rax, prev_regs.rax, "RBX", regs.rbx, prev_regs.rbx);
    print_row_colored("RCX", regs.rcx, prev_regs.rcx, "RDX", regs.rdx, prev_regs.rdx);
    print_row_colored("RSI", regs.rsi, prev_regs.rsi, "RDI", regs.rdi, prev_regs.rdi);

    print_sep();
    if(extended){
        print_extended_registers(&regs);
        print_flags(regs.eflags);
    }
}

static void print_extended_registers(struct user_regs_struct *regs) {
    print_header("EXTENDED REGISTERS (R8–R15)");

    print_row_colored("R8",  regs->r8,  prev_regs.r8, "R9",  regs->r9, prev_regs.r9);
    print_row_colored("R10", regs->r10, prev_regs.r10,"R11", regs->r11, prev_regs.r11);
    print_row_colored("R12", regs->r12, prev_regs.r12, "R13", regs->r13, prev_regs.r13);
    print_row_colored("R14", regs->r14, prev_regs.r14 ,"R15", regs->r15, prev_regs.r15);

    print_sep();
}

static void print_flags(uint64_t rflags) {
    printf("\n+-----------------------------+\n");
    printf("| RFLAGS = 0x%016llx |\n", (unsigned long long)rflags);
    printf("+------+-----+---------------+\n");
    printf("| FLAG | BIT | STATE         |\n");
    printf("+------+-----+---------------+\n");

    printf("| CF   | 0   | %d             |\n", (int)((rflags >> 0) & 1));
    printf("| PF   | 2   | %d             |\n", (int)((rflags >> 2) & 1));
    printf("| AF   | 4   | %d             |\n", (int)((rflags >> 4) & 1));
    printf("| ZF   | 6   | %d             |\n", (int)((rflags >> 6) & 1));
    printf("| SF   | 7   | %d             |\n", (int)((rflags >> 7) & 1));
    printf("| TF   | 8   | %d             |\n", (int)((rflags >> 8) & 1));
    printf("| IF   | 9   | %d             |\n", (int)((rflags >> 9) & 1));
    printf("| DF   | 10  | %d             |\n", (int)((rflags >> 10) & 1));
    printf("| OF   | 11  | %d             |\n", (int)((rflags >> 11) & 1));

    printf("+------+-----+---------------+\n");
}

