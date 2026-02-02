#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <stdint.h>

typedef enum {
    PUSH  = 0x01,
    POP   = 0x02,
    DUP   = 0x03,

    ADD   = 0x10,
    SUB   = 0x11,
    MUL   = 0x12,
    DIV   = 0x13,
    LT    = 0x14,
    LTE   = 0x15,
    GT    = 0x16,
    GTE   = 0x17,
    EQ    = 0x18,

    JMP   = 0x20,
    JZ    = 0x21,
    JNZ   = 0x22,

    STORE = 0x30,
    LOAD  = 0x31,

    CALL  = 0x40,
    RET   = 0x41,

    NEW_PAIR = 0x50,
    SET_LEFT = 0x51,
    SET_RIGHT = 0x52,
    GET_LEFT = 0x53,
    GET_RIGHT = 0x54,
    REMOVE_LEFT = 0x55,
    REMOVE_RIGHT = 0x56,

    MAKE_FUNC = 0x60,
    MAKE_CLOSURE = 0x61,
    CALL_CLOSURE = 0x62,

    GC = 0x70,
    
    PRINT = 0x80,

    BREAK = 0xCC,
    
    HALT  = 0xFF
} opcode;

char *opcode_to_string(unsigned char op);
#endif
