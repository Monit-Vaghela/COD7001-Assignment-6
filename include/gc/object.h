#ifndef OBJECT_H
#define OBJECT_H

#include <gc/value.h>
#include <stdlib.h>
#include <stdio.h>

typedef enum {
    OBJ_PAIR,
    OBJ_FUNC,
    OBJ_CLOSURE
} HeapObjType;

typedef struct HeapObj {
    HeapObjType type;
    char Objname[16];
    int marked;
    int used;
    union {
        struct {
            Value left;
            Value right;
        } pair;

        struct {
            Value entry;
        } func;

        struct {
            Value func;
            Value env;
        } closure;
    } as;
} HeapObj;

typedef struct HeapLinkList {
    HeapObj *node;
    struct HeapLinkList *next;
} HeapLinkList;

/* MUST take Value, not HeapObj* */
HeapObj *new_pair(HeapLinkList **heap_head, Value left, Value right);
HeapObj *new_function(HeapLinkList **heap_head, int entry);
HeapObj *new_closure(HeapLinkList **heap_head, Value func, Value env);
void object_reset_counters(void);

#endif
