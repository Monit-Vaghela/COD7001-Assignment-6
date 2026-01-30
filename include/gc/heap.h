#ifndef HEAP_H
#define HEAP_H

#include <gc/object.h>

/* Head of heap linked list */
extern HeapLinkList *heap_head;

int heap_add(HeapObj *obj);
int heap_count(void);

HeapObj *heap_alloc(void);

Value value_int(int x);
Value value_obj(HeapObj *o);
Value value_null(void);

void print_Value(Value v);
void heap_print(HeapLinkList *head);
#endif
