#ifndef GC_H
#define GC_H

struct vm;          
typedef struct vm vm;

void gc_init(void);
void gc_run(vm *vm);
void gc(vm *vm);

#endif
