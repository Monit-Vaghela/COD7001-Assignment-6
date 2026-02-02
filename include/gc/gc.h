#ifndef GC_H
#define GC_H

struct vm;          
typedef struct vm vm;

void gc_run(vm *vm);
void mark_from_roots(vm *vm);
#endif
