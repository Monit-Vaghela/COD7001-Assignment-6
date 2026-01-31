#include <stdio.h>
#include <VM/memory.h>
// #include <VM/metrics.h>
int mem_error_flag = 0;
Value mem_error;

void memory_init(Value *memory) {
    for (int i = 0; i < MEM_SIZE; i++){
        memory[i].type = VAL_INT;
        memory[i].int_value = 0;
    }
        
}

int memory_store(Value *memory, int addr, Value data){
    // clock_t t1 = clock();
    if(addr >= MEM_SIZE || addr < 0){
        return 1;
    }
    memory[addr] = data;
    // clock_t t2 = clock();
    // memory_time += (t2 - t1);
    
    return 0;
}

Value memory_load(Value *memory, int addr){
    // clock_t t1 = clock();
    if(addr >= MEM_SIZE || addr < 0){
        mem_error_flag = 1;
        return mem_error;
    }
    // clock_t t2 = clock();
    // memory_time += (t2 - t1);
    return memory[addr];
}