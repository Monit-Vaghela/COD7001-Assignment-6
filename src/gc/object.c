#include <gc/object.h>
#include <stdlib.h>
#include <gc/heap.h>

static int pair_counter = 1;
static int function_counter = 1;
static int closure_counter = 1;

HeapObj *new_pair(Value left, Value right) {
    HeapObj *obj = heap_alloc();
    obj->type = OBJ_PAIR;
    
    sprintf(obj->Objname, "P%d", pair_counter++);

    obj->as.pair.left = left;
    obj->as.pair.right = right;

    heap_add(obj);
    return obj;
}

HeapObj *new_function(int entry){
    HeapObj *obj = heap_alloc();
    obj->type = OBJ_FUNC;

    sprintf(obj->Objname, "F%d", function_counter++);

    Value val_entry = value_int(entry);
    obj->as.func.entry = val_entry;

    heap_add(obj);
    return obj;
}

HeapObj *new_closure(Value func, Value env){
    HeapObj *obj = heap_alloc();
    obj->type = OBJ_CLOSURE;
    
    sprintf(obj->Objname, "C%d", closure_counter++);

    obj->as.closure.func = func;
    obj->as.closure.env = env;

    heap_add(obj);
    return obj;
}

void object_reset_counters(void) {
    pair_counter = 1;
    function_counter = 1;
    closure_counter = 1;
}
