#ifndef HEAP_H
#define HEAP_H

#include <gc/object.h>

int heap_add(HeapObj *obj, HeapLinkList **heap_head);
int heap_count(HeapLinkList *heap_head);

HeapObj *heap_alloc(void);

Value value_int(int x);
Value value_obj(HeapObj *o);
Value value_null(void);

void print_Value(Value v);
void heap_print(HeapLinkList *head);
void heap_destroy(HeapLinkList *heap_head);

#endif
