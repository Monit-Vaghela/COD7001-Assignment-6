#include <gc/object.h>
#include <stdlib.h>
#include <gc/heap.h>

static int pair_counter = 1;
static int function_counter = 1;
static int closure_counter = 1;

HeapObj *new_pair(HeapLinkList **heap_head, Value left, Value right) {
    HeapObj *obj = heap_alloc();
    obj->type = OBJ_PAIR;
    obj->used = 0;
    sprintf(obj->Objname, "P%d", pair_counter++);

    obj->as.pair.left = left;
    obj->as.pair.right = right;

    heap_add(obj, heap_head);
    return obj;
}


HeapObj *new_function(HeapLinkList **heap_head, int entry){
    HeapObj *obj = heap_alloc();
    obj->type = OBJ_FUNC;

    sprintf(obj->Objname, "F%d", function_counter++);

    Value val_entry = value_int(entry);
    obj->as.func.entry = val_entry;

    heap_add(obj, heap_head);
    return obj;
}

HeapObj *new_closure(HeapLinkList **heap_head, Value func, Value env){
    HeapObj *obj = heap_alloc();
    obj->type = OBJ_CLOSURE;
    
    sprintf(obj->Objname, "C%d", closure_counter++);

    obj->as.closure.func = func;
    obj->as.closure.env = env;

    heap_add(obj, heap_head);
    return obj;
}

void object_reset_counters(void) {
    pair_counter = 1;
    function_counter = 1;
    closure_counter = 1;
}
