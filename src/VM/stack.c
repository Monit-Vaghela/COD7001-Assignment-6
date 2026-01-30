#include <VM/stack.h>
// #include <VM/metrics.h>
#include <stdlib.h>
#include <stdio.h>

int stck_error_flag = 0;

// defining a NULL Value type to identify some error
Value error;

void stack_init(stack *s) {
    s->top = -1;
}

int stack_push(stack *s, Value value) {
    // clock_t t1 = clock();
    if (stack_isfull(s) != 0) return 1;

    s->data[++s->top] = value;
    // clock_t t2 = clock();
    // stack_time += (t2 - t1);
    return 0;
}

Value stack_pop(stack *s) {
    // clock_t t1 = clock();
    if(stack_isempty(s) != 0) {
        stck_error_flag = 1;
        return error;
    }
    // clock_t t2 = clock();
    // stack_time += (t2 - t1);
    return s->data[s->top--];
}

int stack_isempty(stack *s){
    if(s->top == -1) {
        return 1;
    }
    return 0;
}

int stack_isfull(stack *s){
    if(s->top == STACK_MAX - 1) {
        return 1;
    }
    return 0;
}

void stack_print(stack *s) {
    printf("[");
    for(int i = 0; i <= s->top; i++){
        if(s->data[i].type == VAL_INT){
            printf("VAL_INT(%d), ", s->data[i].int_value);
        }
        else if(s->data[i].type == VAL_OBJ){
            HeapObjType type = s->data[i].obj->type;
            if(type == OBJ_PAIR){
                printf("OBJ_PAIR(%s), ", s->data[i].obj->Objname);
            }
            else if(type == OBJ_FUNC){
                printf("OBJ_FUNC(%s), ", s->data[i].obj->Objname);
            }
            else if(type == OBJ_CLOSURE){
                printf("OBJ_CLOSURE(%s), ", s->data[i].obj->Objname);
            }
        }
        
    }
    printf("]\n");
}
