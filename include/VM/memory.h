#ifndef MEMORY_H
#define MEMORY_H

#define MEM_SIZE 1024
#include <gc/value.h>

extern int mem_error_flag;
void memory_init(Value *memory);
Value memory_load(Value *memory, int addr);
int memory_store(Value *memory, int addr, Value data);

#endif
