#include <gc/heap.h>
#include <gc/object.h>
#include <stdlib.h>

Value value_int(int x) {
    Value v;
    v.type = VAL_INT;
    v.int_value = x;
    return v;
}

Value value_obj(HeapObj *o) {
    Value v;
    v.type = VAL_OBJ;
    v.obj = o;
    return v;
}

Value value_null(void) {
    Value v;
    v.type = VAL_NULL;
    return v;
}

HeapObj *heap_alloc() {
    HeapObj *obj = malloc(sizeof(HeapObj));

    if (!obj) return NULL;
    

    obj->marked = 0;
    return obj;
}
 
void print_Value(Value v) {
    if (v.type == VAL_INT)
        printf("VAL_INT(%d)", v.int_value);
    else if (v.type == VAL_OBJ)
        printf("VAL_OBJ(%s)", v.obj->Objname);
    else
        printf("NULL");
}

int heap_add(HeapObj *obj, HeapLinkList **heap_head) {
    HeapLinkList *node = malloc(sizeof(HeapLinkList));
    if (!node) return 1;

    node->node = obj;
    node->next = *heap_head;
    *heap_head = node;
    return 0;
}


int heap_count(HeapLinkList *heap_head) {
    int count = 0;
    HeapLinkList *curr = heap_head;
    while (curr) {
        count++;
        curr = curr->next;
    }
    return count;
}

void heap_get_leaks(HeapLinkList *heap_head) {
    int leaked_bytes = 0;
    int count = 0;
    HeapLinkList *curr = heap_head;
    while (curr) {
        if(curr->node->used == 0){
            int bytes;
            if(curr->node->type == VAL_INT || curr->node->type == VAL_NULL) bytes = 16;
            else if(curr->node->type == VAL_OBJ) bytes = 64;
            else bytes = 0;
            leaked_bytes += bytes;
            count += 1;
        }
        curr = curr->next;
    }
    printf("Number of Leaked Objects | %d\n", count);
    printf("Leaked Bytets              %d\n", leaked_bytes);
    
}

void heap_print_node(HeapLinkList *object, int index){
    HeapObj *obj = object->node;

    printf("[%d] HeapObj @ %p | Object Name=%s | type=", index, (void *)obj, obj->Objname);

    if (obj->type == OBJ_PAIR) {
        printf("PAIR | marked=%d  | used=%d\n", obj->marked, obj->used);
        printf("     left  = ");
        print_Value(obj->as.pair.left);
        printf("\n     right = ");
        print_Value(obj->as.pair.right);
        printf("\n");
    }
    else if(obj->type == OBJ_FUNC){
        printf("FUNC | marked=%d  | used=%d\n", obj->marked, obj->used);
        printf("     entry  = ");
        print_Value(obj->as.func.entry);
        printf("\n");
    }
    else if(obj->type == OBJ_CLOSURE){
        printf("CLOSURE | marked=%d  | used=%d\n", obj->marked, obj->used);
        printf("     func  = ");
        print_Value(obj->as.closure.func);
        printf("\n     right = ");
        print_Value(obj->as.closure.env);
        printf("\n");
    }
    printf("------------------------------------------------\n");
}

void heap_print(HeapLinkList *head) {
    printf("\n---------------------- HEAP ----------------------\n");

    if (!head) {
        printf("Heap is empty\n");
        return;
    }

    int index = 0;
    while (head) {
        heap_print_node(head, index);
        head = head->next;
        index++;
    }
}

void heap_destroy(HeapLinkList *heap_head) {
    HeapLinkList *curr = heap_head;

    while (curr) {
        HeapLinkList *next = curr->next;
        free(curr->node);   // HeapObj
        free(curr);         // HeapLinkList
        curr = next;
    }

    heap_head = NULL;
}
