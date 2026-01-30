#ifndef VM_H
#define VM_H

#include <VM/stack.h>
#include <VM/memory.h>
#include <VM/instruction.h>
#include <time.h>
#include <gc/value.h>

typedef struct vm {
    unsigned char *code;
    int code_size;
    int pc;
    int running;

    stack stack;
    stack call_stack;
    Value memory[MEM_SIZE];
} vm;

extern clock_t stack_time;
extern clock_t memory_time;
void vm_init(vm *vm, unsigned char *code, int code_size);
void vm_run(vm *vm, int debug, int benchmark);

#endif
