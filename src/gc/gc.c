#include <gc/gc.h>
#include <VM/vm.h>   
#include <gc/heap.h>
#include <gc/object.h>
#include <gc/value.h>
#include <stdlib.h>
// #include <VM/metrics.h>

static HeapObj *value_to_object(Value v) {
    if (v.type == VAL_OBJ) {
        return v.obj;
    }
    return NULL;
}

static void push_children(HeapObj *obj, HeapObj **worklist, int *top) {
    if (!obj) return;

    switch (obj->type) {

        case OBJ_PAIR: {
            HeapObj *left  = value_to_object(obj->as.pair.left);
            HeapObj *right = value_to_object(obj->as.pair.right);

            if (left)  worklist[(*top)++] = left;
            if (right) worklist[(*top)++] = right;
            break;
        }

        case OBJ_CLOSURE: {
            HeapObj *func = value_to_object(obj->as.closure.func);
            HeapObj *env  = value_to_object(obj->as.closure.env);

            if (func) worklist[(*top)++] = func;
            if (env)  worklist[(*top)++] = env;
            break;
        }

        case OBJ_FUNC:
            /* No heap references */
            break;
    }
}

static void mark_from_roots(vm *vm) {
    if (!vm) return;

    int capacity = 1024;
    int top = 0;

    HeapObj **worklist = malloc(sizeof(HeapObj *) * capacity);
    if (!worklist) return;

    /* Root discovery: VM stack */
    for (int i = 0; i <= vm->stack.top; i++) {
        HeapObj *obj = value_to_object(vm->stack.data[i]);
        if (obj) {
            worklist[top++] = obj;
        }
    }

    /* Iterative marking */
    while (top > 0) {
        HeapObj *obj = worklist[--top];

        if (!obj || obj->marked)
            continue;

        obj->marked = 1;
        // gc_marked_count++;
        printf("[GC-MARK] Object %s marked\n", obj->Objname);
        push_children(obj, worklist, &top);
    }

    free(worklist);
}

static void sweep(HeapLinkList **heap_head) {
    HeapLinkList *curr = *heap_head;   
    HeapLinkList *prev = NULL;

    while (curr) {
        HeapObj *obj = curr->node;

        if (!obj->marked) {
            HeapLinkList *garbage = curr;

            if (prev == NULL) {
                /* removing head */
                *heap_head = curr->next;   
                curr = *heap_head;
            } else {
                prev->next = curr->next;
                curr = curr->next;
            }

            free(garbage->node);
            free(garbage);
        }
        else {
            /* reachable */
            obj->marked = 0;
            prev = curr;
            curr = curr->next;
        }
    }
}


void gc_run(vm *vm) {
    printf("\n=== GC START ===\n");
    // clock_ t1 = clock();
    mark_from_roots(vm);
    // clock_ t2 = clock();
    // mark_time += (t2 -t1);
    printf("=== GC END ===\n");  
    // t1 = clock();
    sweep(&vm->heap_head);
    // t2 = clock();
    // ake _time += (t2 -t1);             
}

// void gc(vm *vm) {
//     // gc_runs++;

//     // gc_objects_before += heap_count();
//     gc_run(vm);
//     // gc_objects_after += heap_count();
// }
