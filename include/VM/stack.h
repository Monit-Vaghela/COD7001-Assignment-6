#ifndef STACK_H
#define STACK_H
#include <gc/value.h>
#include <gc/heap.h>

#define STACK_MAX 1024

typedef struct {
    Value data[STACK_MAX];
    int top;
} stack;
extern int stck_error_flag;

void stack_init(stack *s);
int stack_push(stack *s, Value v);
Value stack_pop(stack *s);
int stack_isempty(stack *s);
int stack_isfull(stack *s);
void stack_print(stack *s);

#endif
