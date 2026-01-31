#ifndef MEMORY_H
#define MEMORY_H

#include <sys/types.h>
#include <stddef.h>


void inspect_memory(unsigned long addr, pid_t pid, int count);

#endif /* MEMORY_H */
