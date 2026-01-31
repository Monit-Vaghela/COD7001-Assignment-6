#include <VM/vm.h>
// #include <VM/metrics.h>
#include <stdio.h>
#include <stdint.h>
#include <gc/heap.h>
#include <gc/gc.h>

static int vm_error_flag = 0;
static unsigned char mem_initialized[MEM_SIZE];



static int arithmatic_underflow(vm *vm, char *operator){
    if(vm->stack.top < 1){
        printf("[VM ERROR / %d]: Not enough Operands on stack to perform %s\n", vm->pc, operator);
        vm->running = 0;
        return 1;
    }
    return 0;
}

static int check_next_4byte(vm *vm, char *opcode){
    if(vm->pc + 4 >= vm->code_size){
        printf("[VM Error / %d]: %s Requires 4 Operand bytes\n", vm->pc, opcode);
        vm->running = 0;
        vm_error_flag = 1;
        return 0;
    }
    int32_t value = 0;
    for (int i = 1; i <= 4; i++) {
        value = (value << 8) | vm->code[vm->pc + i];
    }
    return value;
}

static int check_loop(vm *vm, char *loop_type){
    int addr = check_next_4byte(vm, loop_type);
    if(vm_error_flag == 1) return 0;

    if (addr >= vm->code_size) {
        printf("[VM Error / %d]: Out of Bound Address\n", vm->pc);
        vm->running = 0;
        vm_error_flag = 1;
        return 0;
    }
    return addr;
}

void print_mem(vm *vm){
    int found = 0;
    for(int i = 0; i < MEM_SIZE; i++){
        if(mem_initialized[i]){
            if(vm->memory[i].type == VAL_INT){
                printf("[ADDR %3d] = VAL_INT(%d)\n", i, vm->memory[i].int_value);
            }
            else if(vm->memory[i].type == VAL_OBJ){
                HeapObjType type = vm->memory[i].obj->type;
                if(type == OBJ_PAIR){
                    printf("[ADDR %3d] = OBJ_PAIR(%s)\n", i, vm->memory[i].obj->Objname);
                }
                else if(type == OBJ_FUNC){
                    printf("[ADDR %3d] = OBJ_FUNC(%s)\n", i, vm->memory[i].obj->Objname);
                }
                else if(type == OBJ_CLOSURE){
                    printf("[ADDR %3d] = OBJ_CLOSURE(%s)\n", i, vm->memory[i].obj->Objname);
                }
            }
            
            found = 1;
        }
    }
    if(!found) printf("NO Initialized Memory\n");
}

void print_vm_state(vm *vm, unsigned char opcode){
    printf("\n====================================================\n");
    printf("                 VIRTUAL MACHINE STATE               \n");
    printf("====================================================\n");

    printf("PC      : %-4d\n", vm->pc);
    printf("OPCODE  : 0x%02X\n", opcode);
    printf("RUNNING : %s\n", vm->running ? "YES" : "NO");

    printf("\n---------------------- STACK ------------------------\n");
    stack_print(&vm->stack);

    printf("\n------------------- CALL STACK ----------------------\n");
    stack_print(&vm->call_stack);

    printf("\n---------------------- MEMORY -----------------------\n");
    print_mem(vm);

    heap_print(heap_head);
    printf("====================================================\n");
}

void vm_init(vm *vm, unsigned char *code, int code_size){
    // printf("Initializing virtual machine\n");
    vm->code = code;
    vm->code_size = code_size;
    vm->pc = 0;
    
    vm->running = 1;
    
    stack_init(&vm->stack);
    
    stack_init(&vm->call_stack);
    
    memory_init(vm->memory);
    
    for(int i = 0; i < MEM_SIZE; i++)
        mem_initialized[i] = 0;
}

void vm_reset(vm *vm) {
    if (!vm) {
        printf("NO VM\n");
        return;
    }

    /* Execution state */
    vm->pc = 0;
    vm->running = 1;

    /* Stacks */
    stack_init(&vm->stack);
    stack_init(&vm->call_stack);

    /* Memory */
    memory_init(vm->memory);
    for (int i = 0; i < MEM_SIZE; i++) {
        mem_initialized[i] = 0;
    }

    /* Heap */
    heap_destroy();

    /* Object naming reset (important for debugger determinism) */
    object_reset_counters();
}

int vm_run(vm *vm, int debug, int benchmark, int stepSize) {
    while(vm->running && vm->pc < vm->code_size){
        unsigned char opcode = vm->code[vm->pc];

        if(benchmark) printf("-");
        if(debug) print_vm_state(vm, opcode);
        // printf("Current Instruction : %02x\n", opcode);

        switch (opcode){
            case PUSH:{
                int operand = check_next_4byte(vm, "PUSH");
                if(vm_error_flag == 1) break;

                // Create a Operand value
                Value valueObj = value_int(operand);

                if(stack_push(&vm->stack, valueObj) != 0){
                    printf("[VM Error / %d]: Stack is Full Cannot Push\n", vm->pc);
                    vm->running = 0;
                    break;
                }
                (vm->pc) += 5;
                break;
            }
            
            case POP:{
                stack_pop(&vm->stack);
                if(stck_error_flag){
                    printf("[VM Error / %d]: Stack Is Empty Cannot Pop\n", vm->pc);
                    vm->running = 0;
                    break;
                }
                (vm->pc)++;
                break;
            }

            case DUP:{
                if(stack_isempty(&vm->stack) != 0){
                    printf("[VM ERROR / %d]: Cannot Duplicate Top of Stack\n", vm->pc);
                    vm->running = 0;
                    break;
                }

                Value top_value = vm->stack.data[vm->stack.top];

                if(top_value.type == VAL_INT || top_value.type == VAL_OBJ){
                    if(stack_push(&vm->stack, top_value) != 0){
                        printf("[VM Error / %d]: Stack is Full Cannot Push\n", vm->pc);
                        vm->running = 0;
                        break;
                    }
                }
                
                (vm->pc)++;
                break;
            }

            case ADD:{
                if(arithmatic_underflow(vm, "ADD") != 0) break;
                Value b = stack_pop(&vm->stack);
                Value a = stack_pop(&vm->stack);
                Value c;
                int c_value;

                if(a.type == VAL_INT && b.type == VAL_INT){
                    // clock_t t1 = clock();
                    c_value = a.int_value + b.int_value;
                    // clock_t t2 = clock();
                    // alu_time += (t2 - t1);
                    c = value_int(c_value);
                }
                else {
                    printf("[VM ERROR / %d] Arithmetic operation can be applied to Heap Object\n", vm->pc);
                    vm -> running = 0;
                    break;
                }

                stack_push(&vm->stack, c);
                (vm->pc)++;
                break;
            }

            case SUB:{
                if(arithmatic_underflow(vm, "SUB") != 0) break;

                Value b = stack_pop(&vm->stack);
                Value a = stack_pop(&vm->stack);
                Value c;
                int c_value;

                if(a.type == VAL_INT && b.type == VAL_INT){
                    // clock_t t1 = clock();
                    c_value = a.int_value - b.int_value;
                    // clock_t t2 = clock();
                    // alu_time += (t2 - t1);

                    c = value_int(c_value);
                }
                else {
                    printf("[VM ERROR / %d] Arithmetic operation can be applied to Heap Object\n", vm->pc);
                    vm -> running = 0;
                    break;
                }
            
                stack_push(&vm->stack, c);
                (vm->pc)++;
                break;
            }

            case MUL:{
                if(arithmatic_underflow(vm, "MUL") != 0) break;

                Value b = stack_pop(&vm->stack);
                Value a = stack_pop(&vm->stack);
                Value c;
                int c_value;

                if(a.type == VAL_INT && b.type == VAL_INT){
                    // clock_t t1 = clock();
                    c_value = a.int_value * b.int_value;
                    // clock_t t2 = clock();
                    // alu_time += (t2 - t1);
                    c = value_int(c_value);
                }
                else {
                    printf("[VM ERROR / %d] Arithmetic operation can be applied to Heap Object\n", vm->pc);
                    vm -> running = 0;
                    break;
                }

                stack_push(&vm->stack, c);
                (vm->pc)++;
                break;
            }

            case DIV:{
                if(arithmatic_underflow(vm, "DIV") != 0) break;

                Value b = stack_pop(&vm->stack);
                if(b.type == VAL_INT && b.int_value == 0){
                    printf("[VM Error / %d] Denominator cannot be 0\n", vm->pc);
                    vm->running = 0;
                    break;
                }

                Value a = stack_pop(&vm->stack);
                Value c;
                int c_value;

                if(a.type == VAL_INT && b.type == VAL_INT){
                    // clock_t t1 = clock();
                    c_value = a.int_value / b.int_value;
                    // clock_t t2 = clock();
                    // alu_time += (t2 - t1);
                    c = value_int(c_value);
                }
                else {
                    printf("[VM ERROR / %d] Arithmetic operation can be applied to Heap Object\n", vm->pc);
                    vm -> running = 0;
                    break;
                }

                stack_push(&vm->stack, c);
                (vm->pc)++;
                break;
            }

            case LT:{
                if(arithmatic_underflow(vm, "LT") != 0) break;
                Value b = stack_pop(&vm->stack);
                Value a = stack_pop(&vm->stack);
                
                if(a.type == VAL_INT && b.type == VAL_INT){
                    // clock_t t1 = clock();
                    Value valObj;
                    valObj.type = VAL_INT;
                    if(a.int_value < b.int_value){
                        // clock_t t2 = clock();
                        // alu_time += (t2 - t1);
                        valObj.int_value = 1;
                        stack_push(&vm->stack, valObj);
                    }else{
                        // clock_t t2 = clock();
                        // alu_time += (t2 - t1);
                        valObj.int_value = 0;
                        stack_push(&vm->stack, valObj);
                    }
                }
                else {
                    printf("[VM ERROR / %d] Arithmetic operation can be applied to Heap Object\n", vm->pc);
                    vm -> running = 0;
                    break;
                }

                (vm->pc)++;
                break;
            }

            case GT:{
                if(arithmatic_underflow(vm, "GT") != 0) break;
                Value b = stack_pop(&vm->stack);
                Value a = stack_pop(&vm->stack);
                
                if(a.type == VAL_INT && b.type == VAL_INT){
                    // clock_t t1 = clock();
                    Value valObj;
                    valObj.type = VAL_INT;
                    if(a.int_value > b.int_value){
                        // clock_t t2 = clock();
                        // alu_time += (t2 - t1);
                        valObj.int_value = 1;
                        stack_push(&vm->stack, valObj);
                    }else{
                        // clock_t t2 = clock();
                        // alu_time += (t2 - t1);
                        valObj.int_value = 0;
                        stack_push(&vm->stack, valObj);
                    }
                }
                else {
                    printf("[VM ERROR / %d] Arithmetic operation can be applied to Heap Object\n", vm->pc);
                    vm -> running = 0;
                    break;
                }

                (vm->pc)++;
                break;
            }

            case LTE:{
                if(arithmatic_underflow(vm, "LTE") != 0) break;
                Value b = stack_pop(&vm->stack);
                Value a = stack_pop(&vm->stack);
                
                if(a.type == VAL_INT && b.type == VAL_INT){
                    // clock_t t1 = clock();
                    Value valObj;
                    valObj.type = VAL_INT;
                    if(a.int_value <= b.int_value){
                        // clock_t t2 = clock();
                        // alu_time += (t2 - t1);
                        valObj.int_value = 1;
                        stack_push(&vm->stack, valObj);
                    }else{
                        // clock_t t2 = clock();
                        // alu_time += (t2 - t1);
                        valObj.int_value = 0;
                        stack_push(&vm->stack, valObj);
                    }
                }
                else {
                    printf("[VM ERROR / %d] Arithmetic operation can be applied to Heap Object\n", vm->pc);
                    vm -> running = 0;
                    break;
                }

                (vm->pc)++;
                break;
            }

            case GTE:{
                if(arithmatic_underflow(vm, "GTE") != 0) break;
                Value b = stack_pop(&vm->stack);
                Value a = stack_pop(&vm->stack);
                
                if(a.type == VAL_INT && b.type == VAL_INT){
                    // clock_t t1 = clock();
                    Value valObj;
                    valObj.type = VAL_INT;
                    if(a.int_value >= b.int_value){
                        // clock_t t2 = clock();
                        // alu_time += (t2 - t1);
                        valObj.int_value = 1;
                        stack_push(&vm->stack, valObj);
                    }else{
                        // clock_t t2 = clock();
                        // alu_time += (t2 - t1);
                        valObj.int_value = 0;
                        stack_push(&vm->stack, valObj);
                    }
                }
                else {
                    printf("[VM ERROR / %d] Arithmetic operation can be applied to Heap Object\n", vm->pc);
                    vm -> running = 0;
                    break;
                }

                (vm->pc)++;
                break;
            }

            case EQ:{
                if(arithmatic_underflow(vm, "EQ") != 0) break;
                Value b = stack_pop(&vm->stack);
                Value a = stack_pop(&vm->stack);
                
                if(a.type == VAL_INT && b.type == VAL_INT){
                    // clock_t t1 = clock();
                    Value valObj;
                    valObj.type = VAL_INT;
                    if(a.int_value == b.int_value){
                        // clock_t t2 = clock();
                        // alu_time += (t2 - t1);
                        valObj.int_value = 1;
                        stack_push(&vm->stack, valObj);
                    }else{
                        // clock_t t2 = clock();
                        // alu_time += (t2 - t1);
                        valObj.int_value = 0;
                        stack_push(&vm->stack, valObj);
                    }
                }
                else {
                    printf("[VM ERROR / %d] Arithmetic operation can be applied to Heap Object\n", vm->pc);
                    vm -> running = 0;
                    break;
                }

                (vm->pc)++;
                break;
            }

            case JMP:{
                // clock_t t1 = clock();
                int addr = check_loop(vm, "JMP");
                if(vm_error_flag != 0) break;
                if (debug) printf("JUMPING TO %d\n", addr);
                vm->pc = addr;
                // clock_t t2 = clock();
                // control_time += (t2 - t1);
                break;     
            }

            case JZ:{
                // clock_t t1 = clock();
                Value top = stack_pop(&vm->stack);
                if(top.type == VAL_OBJ){
                    printf("[VM Error / %d]: JZ Only Operates on Integer Values\n", vm->pc);
                    vm->running = 0;
                    break;
                }

                if (stck_error_flag) {
                    printf("[VM Error / %d]: Stack is empty\n", vm->pc);
                    vm->running = 0;
                    break;
                }

                int addr = check_loop(vm, "JZ");
                if(vm_error_flag != 0) break;

                if (top.int_value == 0) {
                    if (debug) printf("JUMPING TO %d\n", addr);
                    vm->pc = addr;
                } else {
                    vm->pc += 5;
                }
                // clock_t t2 = clock();
                // control_time += (t2 - t1);
                break;
            }

            case JNZ:{
                // clock_t t1 = clock();
                Value top = stack_pop(&vm->stack);

                if(top.type == VAL_OBJ){
                    printf("[VM Error / %d]: JNZ Only Operates on Integer Values\n", vm->pc);
                    vm->running = 0;
                    break;
                }

                if (stck_error_flag) {
                    printf("[VM Error / %d]: Stack is empty\n", vm->pc);
                    vm->running = 0;
                    break;
                }

                int addr = check_loop(vm, "JNZ");
                if(vm_error_flag != 0) break;

                if (top.int_value != 0) {
                    if (debug) printf("JUMPING TO %d\n", addr);
                    vm->pc = addr;
                } else {
                    vm->pc += 5;
                }
                // clock_t t2 = clock();
                // control_time += (t2 - t1);
                break;
            }

            case STORE:{
                int addr = check_next_4byte(vm, "STORE");
                if(vm_error_flag == 1) break;

                Value top = stack_pop(&vm->stack);
                // printf("TYPE OF TOP IS %d\n", top.type);
                if (stck_error_flag) {
                    printf("[VM Error / %d]: Stack is empty\n", vm->pc);
                    vm->running = 0;
                    break;
                }
                if(memory_store(vm->memory, addr, top) != 0){
                    printf("[VM Error / %d] %d Address is Out of Bounds", vm->pc, addr);
                    vm->running = 0;
                    break;
                }
                mem_initialized[addr] = 1;

                if (debug) {
                    printf("DATA STORED AT %d is", addr);
                    print_Value(top);
                    printf("\n");
                }

                (vm->pc) += 5;
                
                break;
            }

            case LOAD:{
                int addr = check_next_4byte(vm, "LOAD");
                if(vm_error_flag == 1) break;

                Value data = memory_load(vm->memory, addr);
                if(mem_error_flag != 0){
                    printf("[VM Error / %d] %d Address is Out of Bounds", vm->pc, addr);
                    vm->running = 0;
                    break;
                }
                stack_push(&vm->stack, data);

                if (debug) {
                    printf("DATA ");
                    print_Value(data);
                    printf(" Loaded from %d\n", addr);
                }
                (vm->pc) += 5;
                break;
            }

            case CALL:{
                // clock_t t1 = clock();
                int addr = check_next_4byte(vm, "CALL");
                if(vm_error_flag == 1) break;

                if(addr >= vm->code_size){
                    printf("[VM Error / %d]: Call Address %d is out of bounds\n", vm->pc, addr);
                    vm->running = 0;
                    break;
                }

                Value retAddr = value_int(vm->pc + 5);

                if (stack_push(&vm->call_stack, retAddr) != 0) {
                    printf("[VM Error / %d]: Call Stack is Full\n", vm->pc);
                    vm->running = 0;
                    break;
                }

                vm->pc = addr;
                // clock_t t2 = clock();
                // control_time += (t2 - t1);
                break;
            }

            case RET:{
                // clock_t t1 = clock();
                Value ret_addr = stack_pop(&vm->call_stack);
                if(ret_addr.type == VAL_OBJ){
                    printf("[VM Error / %d]: Return Address Cannot be Heap Object\n", vm->pc);
                    vm->running = 0;
                    break;
                }

                if(stck_error_flag != 0){
                    printf("[VM Error / %d]: No Return Value present in call stack\n", vm->pc);
                    vm->running = 0;
                    break;
                }
                if(ret_addr.int_value >= vm->code_size){
                    printf("[VM Error / %d]: Return Address %d is out of Bounds\n", vm->pc, ret_addr.int_value);
                    vm->running = 0;
                    break;
                }
                vm->pc = ret_addr.int_value;
                // clock_t t2 = clock();
                // control_time += (t2 - t1);
                break;
            }

            case NEW_PAIR:{
                Value left = value_null();
                Value right = value_null();

                HeapObj *obj = new_pair(left, right);
                Value v = value_obj(obj);
                if(stack_push(&vm->stack, v) != 0){
                    printf("[VM Error / %d]: Stack is Full Cannot Push\n", vm->pc);
                    vm->running = 0;
                    break;
                }

                (vm->pc)++;
                break;
            }

            case SET_LEFT:{
                if(arithmatic_underflow(vm, "SET_LEFT") != 0) break;
                Value b = stack_pop(&vm->stack);
                Value a = stack_pop(&vm->stack);

                if(a.type != VAL_OBJ){
                    printf("[VM ERROR / %d] Parent Must be a HeapObj to perform setting Operation\n", vm->pc);
                    vm->running = 0;
                    break;
                }

                // Right now this makes no sense as we only have OBJ_PAIR
                if(a.obj->type != OBJ_PAIR){
                    printf("[VM ERROR / %d]: SET_LEFT can only be applied to PAIR object\n", vm->pc);
                    vm->running = 0;
                    break;
                }

                // Assign b to left of a

                a.obj->as.pair.left = b;
                
                if(stack_push(&vm->stack, a) != 0){
                    printf("[VM Error / %d]: Stack is Full Cannot Push\n", vm->pc);
                    vm->running = 0;
                    break;
                }

                (vm->pc)++;
                break;
            }

            case SET_RIGHT:{
                if(arithmatic_underflow(vm, "SET_RIGHT") != 0) break;
                Value b = stack_pop(&vm->stack);
                Value a = stack_pop(&vm->stack);

                if(a.type != VAL_OBJ){
                    printf("[VM ERROR / %d] Parent Must be a HeapObj to perform setting Operation\n", vm->pc);
                    vm->running = 0;
                    break;
                }

                // Right now this makes no sense as we only have OBJ_PAIR
                if(a.obj->type != OBJ_PAIR){
                    printf("[VM ERROR / %d]: SET_RIGHT can only be applied to PAIR object\n", vm->pc);
                    vm->running = 0;
                    break;
                }
                
                // Assign b to left of a

                a.obj->as.pair.right = b;
                
                if(stack_push(&vm->stack, a) != 0){
                    printf("[VM Error / %d]: Stack is Full Cannot Push\n", vm->pc);
                    vm->running = 0;
                    break;
                }

                (vm->pc)++;
                break;
            }

            case GET_LEFT:{
                Value a = stack_pop(&vm->stack);

                if(a.type != VAL_OBJ){
                    printf("[VM Error / %d]:  GET_LEFT requires heap object as parent\n", vm->pc);
                    vm->running = 0;
                    break;
                }

                if(a.obj->type != OBJ_PAIR){
                    printf("[VM ERROR / %d]: GET_LEFT can only be applied to PAIR object\n", vm->pc);
                    vm->running = 0;
                    break;
                }

                Value left = a.obj->as.pair.left;

                if(stack_push(&vm->stack, left) != 0){
                    printf("[VM Error / %d]: Stack is Full Cannot Push\n", vm->pc);
                    vm->running = 0;
                    break;
                }

                (vm->pc)++;
                break;
            }

            case GET_RIGHT:{
                Value a = stack_pop(&vm->stack);

                if(a.type != VAL_OBJ){
                    printf("[VM Error / %d]:  GET_RIGHT requires heap object as parent\n", vm->pc);
                    vm->running = 0;
                    break;
                }

                if(a.obj->type != OBJ_PAIR){
                    printf("[VM ERROR / %d]: GET_RIGHT can only be applied to PAIR object\n", vm->pc);
                    vm->running = 0;
                    break;
                }

                Value right = a.obj->as.pair.right;

                if(stack_push(&vm->stack, right) != 0){
                    printf("[VM Error / %d]: Stack is Full Cannot Push\n", vm->pc);
                    vm->running = 0;
                    break;
                }

                (vm->pc)++;
                break;
            }

            case REMOVE_LEFT:{
                Value a = stack_pop(&vm->stack);

                // Check of HeapObj
                if(a.type != VAL_OBJ){
                    printf("[VM ERROR / %d] Top Of stack must be VAL_OBJ for REMOVE_LEFT\n", vm->pc);
                    vm->running = 0;
                    break;
                }

                // Check if the TOS is CLOSURE Object
                if(a.obj->type != OBJ_CLOSURE){
                    printf("[VM ERROR / %d] Top Of stack must be OBJ_PAIR for REMOVE_LEFT\n", vm->pc);
                    vm->running = 0;
                    break;
                }

                // If it is NULL dont need to do anything
                if(a.obj->as.pair.left.type == VAL_NULL) break;

                a.obj->as.pair.left = value_null();

                (vm->pc)++;
                break;
            }

            case REMOVE_RIGHT:{
                Value a = stack_pop(&vm->stack);

                // Check of HeapObj
                if(a.type != VAL_OBJ){
                    printf("[VM ERROR / %d] Top Of stack must be VAL_OBJ for REMOVE_LEFT\n", vm->pc);
                    vm->running = 0;
                    break;
                }

                // Check if the TOS is CLOSURE Object
                if(a.obj->type != OBJ_CLOSURE){
                    printf("[VM ERROR / %d] Top Of stack must be OBJ_PAIR for REMOVE_LEFT\n", vm->pc);
                    vm->running = 0;
                    break;
                }

                // If it is NULL dont need to do anything
                if(a.obj->as.pair.right.type == VAL_NULL) break;

                a.obj->as.pair.right = value_null();

                (vm->pc)++;
                break;
            }

            case MAKE_FUNC:{
                int addr = check_next_4byte(vm, "MAKE_FUNC");
                if(vm_error_flag != 0) break;

                if(addr >= vm->code_size){
                    printf("[VM Error / %d]: Call Address %d is out of bounds\n", vm->pc, addr);
                    vm->running = 0;
                    break;
                }

                HeapObj *function = new_function(addr);
                Value v = value_obj(function);
                if(stack_push(&vm->stack, v) != 0){
                    printf("[VM Error / %d]: Stack is Full Cannot Push\n", vm->pc);
                    vm->running = 0;
                    break;
                }

                (vm->pc) += 5;
                break;
            }

            case MAKE_CLOSURE:{
                if(arithmatic_underflow(vm, "MAKE_CLOSURE") != 0) break;
                Value b = stack_pop(&vm->stack);
                Value a = stack_pop(&vm->stack);
                
                // Check if a and b both are VAL_OBJ
                if(a.type != VAL_OBJ || b.type != VAL_OBJ){
                    printf("[VM ERROR / %d] MAKE_CLOSURE works with 2 VAL_OBJ\n", vm->pc);
                    vm->running = 0;
                    break;
                }

                // Check if a is OBJ_PAIR -> i.e env
                if(a.obj->type != OBJ_PAIR){
                    printf("[VM Error / %d] Closure.env Must be of Type OBJ_PAIR", vm->pc);
                    vm->running = 0;
                    break;
                }

                // Check if B is OBJ_FUNC 
                if(b.obj->type != OBJ_FUNC){
                    printf("[VM Error / %d] Closure.func Must be of Type OBJ_FUNC", vm->pc);
                    vm->running = 0;
                    break;
                }

                // Create a new HeapObj of type OBJ_CLOSURE
                HeapObj *closure = new_closure(b, a);
                Value v = value_obj(closure);
                if(stack_push(&vm->stack, v) != 0){
                    printf("[VM Error / %d]: Stack is Full Cannot Push\n", vm->pc);
                    vm->running = 0;
                    break;
                }

                (vm->pc)++;
                break;
            }

            case CALL_CLOSURE:{
                Value a = stack_pop(&vm->stack);

                // Check of HeapObj
                if(a.type != VAL_OBJ){
                    printf("[VM ERROR / %d] Top Of stack must be VAL_OBJ for CALL_CLOSURE\n", vm->pc);
                    vm->running = 0;
                    break;
                }

                // Check if the TOS is CLOSURE Object
                if(a.obj->type != OBJ_CLOSURE){
                    printf("[VM ERROR / %d] Top Of stack must be OBJ_CLOSURE for CALL_CLOSURE\n", vm->pc);
                    vm->running = 0;
                    break;
                }

                Value retAddr = value_int(vm->pc + 5);

                // Push the return address to call_stack
                if (stack_push(&vm->call_stack, retAddr) != 0) {
                    printf("[VM Error / %d]: Call Stack is Full\n", vm->pc);
                    vm->running = 0;
                    break;
                }

                Value func = a.obj->as.closure.func;
                Value env = a.obj->as.closure.env;
                // Push the function environment on top of stack
                if (stack_push(&vm->stack, env) != 0) {
                    printf("[VM Error / %d]: Call Stack is Full\n", vm->pc);
                    vm->running = 0;
                    break;
                }
                vm->pc = func.obj->as.func.entry.int_value;
                break;
            }

            case GC: {
                gc(vm);
                vm->pc += 1;
                break;
            }
        
            case BREAK: {
                // printf("RETURNING FROM HITTING A BREAK POINT.\n");
                return 101;
            }

            case HALT:{
                vm->running = 0;
                break;
            }

            default:
                printf("[VM Error / %d] : Invalid opcode %02x\n", vm->pc, opcode);
                vm->running = 0;
                break;
            }
        if(stepSize == -1){
            // printf("Returning after performing single step\n");
            return 102;
        }
    }
    // printf("\n\nFINAL VM STATE\n");
    // print_vm_state(vm, 0xFF);
    return 100;
    // heap_print(heap_head);
}
