#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#include "assembler/assembler.h"
#include "VM/instruction.h"

#define MAX_LABELS 128
#define INITIAL_CAPACITY 256

typedef struct {
    char name[32];
    int addr;
} label_t;

static label_t labels[MAX_LABELS];
static int label_count = 0;

static void emit_u8(unsigned char **buf, int *size, int *cap, uint8_t v) {
    if (*size + 1 > *cap) {
        *cap *= 2;
        *buf = realloc(*buf, *cap);
        if (!*buf) {
            printf("[Assembler Error] Memory allocation failed\n");
            exit(1);
        }
    }
    (*buf)[(*size)++] = v;
}

static void emit_i32(unsigned char **buf, int *size, int *cap, int32_t v) {
    for (int byte = 3; byte >= 0; byte--) {
        uint8_t part = (v >> (byte * 8)) & 0xFF;
        emit_u8(buf, size, cap, part);
    }
}

static int find_label(const char *name) {
    for (int i = 0; i < label_count; i++) {
        if (strcmp(labels[i].name, name) == 0)
            return labels[i].addr;
    }
    return -1;
}

static int write_bin(const char *asm_file, unsigned char *code, int size) {
    char out_name[256];
    const char *dot = strrchr(asm_file, '.');

    if (dot && strcmp(dot, ".asm") == 0) {
        size_t len = dot - asm_file;
        snprintf(out_name, sizeof(out_name), "%.*s.bin", (int)len, asm_file);
    } else {
        snprintf(out_name, sizeof(out_name), "%s.bin", asm_file);
    }

    FILE *out = fopen(out_name, "wb");
    if (!out) {
        perror("Assembler: output file");
        return -1;
    }

    fwrite(code, 1, size, out);
    fclose(out);
    return 0;
}

// Helper to get next line from string
static const char *get_line(const char *src, char *line, size_t max_len) {
    if (!src || !*src) return NULL;
    
    size_t i = 0;
    while (*src && *src != '\n' && i < max_len - 1) {
        line[i++] = *src++;
    }
    line[i] = '\0';
    
    // Skip the newline
    if (*src == '\n') src++;
     
    return src;
}

// Core assembly logic - works on string input
int assemble(const char *src, unsigned char **out_code) {
    if (!src || !out_code) return -1;
    
    char line[256];
    const char *p = src;
    
    /* PASS 1: Collect labels and calculate addresses */
    int pc = 0;
    label_count = 0;
    
    while (*p) {
        p = get_line(p, line, sizeof(line));
        if (!p && line[0] == '\0') break;
        
        // Skip empty lines and comments
        if (line[0] == '\0' || line[0] == '#' || line[0] == ';')
            continue;
        
        // Check for label
        char *colon = strchr(line, ':');
        if (colon) {
            *colon = '\0';
            // Trim whitespace
            char *label_start = line;
            while (*label_start && isspace(*label_start)) label_start++;
            
            if (label_count >= MAX_LABELS) {
                printf("[Assembler Error] Too many labels\n");
                return -1;
            }
            
            strncpy(labels[label_count].name, label_start, 31);
            labels[label_count].name[31] = '\0';
            labels[label_count].addr = pc;
            label_count++;
            continue;
        }
        
        // Parse instruction
        char instr[32], operand[32];
        int args = sscanf(line, "%31s %31s", instr, operand);
        
        if (args == 2)
            pc += 5;  // opcode + int32
        else if (args == 1)
            pc += 1;  // opcode only
    }
    
    /* PASS 2: Generate bytecode */
    int capacity = INITIAL_CAPACITY;
    int size = 0;
    unsigned char *code = malloc(capacity);
    if (!code) return -1;
    
    p = src;
    
    while (*p) {
        p = get_line(p, line, sizeof(line));
        if (!p && line[0] == '\0') break;
        
        // Skip empty lines, comments, and labels
        if (line[0] == '\0' || line[0] == '#' || line[0] == ';')
            continue;
        if (strchr(line, ':'))
            continue;
        
        char instr[32], operand[32];
        int args = sscanf(line, "%31s %31s", instr, operand);
        
        // Instructions with operand
        if (args == 2) {
            int value;
            
            // Check if operand is a number or label
            if (isdigit(operand[0]) || operand[0] == '-') {
                value = atoi(operand);
            } else {
                value = find_label(operand);
                if (value < 0) {
                    printf("[Assembler Error] Unknown label: %s\n", operand);
                    free(code);
                    return -1;
                }
            }
            // printf("%s \n", instr);
            if (!strcmp(instr, "PUSH")) {
                emit_u8(&code, &size, &capacity, PUSH);
                emit_i32(&code, &size, &capacity, value);
            } else if (!strcmp(instr, "LOAD")) {
                emit_u8(&code, &size, &capacity, LOAD);
                emit_i32(&code, &size, &capacity, value);
            } else if (!strcmp(instr, "STORE")) {
                emit_u8(&code, &size, &capacity, STORE);
                emit_i32(&code, &size, &capacity, value);
            } else if (!strcmp(instr, "JMP")) {
                emit_u8(&code, &size, &capacity, JMP);
                emit_i32(&code, &size, &capacity, value);
            } else if (!strcmp(instr, "JZ")) {
                emit_u8(&code, &size, &capacity, JZ);
                emit_i32(&code, &size, &capacity, value);
            } else if (!strcmp(instr, "JNZ")) {
                emit_u8(&code, &size, &capacity, JNZ);
                emit_i32(&code, &size, &capacity, value);
            } else if (!strcmp(instr, "CALL")) {
                emit_u8(&code, &size, &capacity, CALL);
                emit_i32(&code, &size, &capacity, value);
            } else {
                printf("[Assembler Error] Unknown instruction: %s\n", instr);
                free(code);
                return -1;
            }
        }
        // Instructions without operand
        else if (args == 1) {
            if (!strcmp(instr, "POP")) {
                emit_u8(&code, &size, &capacity, POP);
            } else if (!strcmp(instr, "DUP")) {
                emit_u8(&code, &size, &capacity, DUP);
            } else if (!strcmp(instr, "ADD")) {
                emit_u8(&code, &size, &capacity, ADD);
            } else if (!strcmp(instr, "SUB")) {
                emit_u8(&code, &size, &capacity, SUB);
            } else if (!strcmp(instr, "MUL")) {
                emit_u8(&code, &size, &capacity, MUL);
            } else if (!strcmp(instr, "DIV")) {
                emit_u8(&code, &size, &capacity, DIV);
            } else if (!strcmp(instr, "MUL")) {
                emit_u8(&code, &size, &capacity, MUL);
            } else if (!strcmp(instr, "LT")) {
                emit_u8(&code, &size, &capacity, LT);
            } else if (!strcmp(instr, "GT")) {
                emit_u8(&code, &size, &capacity, GT);
            } else if (!strcmp(instr, "LTE")) {
                emit_u8(&code, &size, &capacity, LTE);
            } else if (!strcmp(instr, "GTE")) {
                emit_u8(&code, &size, &capacity, GTE);
            } else if (!strcmp(instr, "EQ")) {
                emit_u8(&code, &size, &capacity, EQ);
            } else if (!strcmp(instr, "RET")) {
                emit_u8(&code, &size, &capacity, RET);
            } else if (!strcmp(instr, "NEW_PAIR")) {
                emit_u8(&code, &size, &capacity, NEW_PAIR);
            } else if (!strcmp(instr, "SET_LEFT")) {
                emit_u8(&code, &size, &capacity, SET_LEFT);
            } else if (!strcmp(instr, "SET_RIGHT")) {
                emit_u8(&code, &size, &capacity, SET_RIGHT);
            } else if (!strcmp(instr, "GET_LEFT")) {
                emit_u8(&code, &size, &capacity, GET_LEFT);
            } else if (!strcmp(instr, "GET_RIGHT")) {
                emit_u8(&code, &size, &capacity, GET_RIGHT);
            } else if (!strcmp(instr, "REMOVE_LEFT")) {
                emit_u8(&code, &size, &capacity, REMOVE_LEFT);
            } else if (!strcmp(instr, "REMOVE_RIGHT")) {
                emit_u8(&code, &size, &capacity, REMOVE_LEFT);
            }else if (!strcmp(instr, "PRINT")) {
                emit_u8(&code, &size, &capacity, PRINT);
            }else if (!strcmp(instr, "HALT")) {
                emit_u8(&code, &size, &capacity, HALT);
            } else {
                printf("[Assembler Error] Unknown instruction: %s\n", instr);
                free(code);
                return -1;
            }
        }
    }
    
    *out_code = code;
    return size;
}

// // File-based assembly - reads file and calls string-based assembler
// int assemble(const char *input_file, unsigned char **out_code) {
//     FILE *in = fopen(input_file, "r");
//     if (!in) {
//         perror("Assembler: input file");
//         return -1;
//     }
    
//     // Read entire file into string
//     fseek(in, 0, SEEK_END);
//     long file_size = ftell(in);
//     fseek(in, 0, SEEK_SET);
    
//     char *src = malloc(file_size + 1);
//     if (!src) {
//         fclose(in);
//         return -1;
//     }
    
//     size_t read_size = fread(src, 1, file_size, in);
//     src[read_size] = '\0';
//     fclose(in);
    
//     // Assemble from string
//     unsigned char *code = NULL;
//     int size = assemble_from_string(src, &code);
//     free(src);
    
//     if (size < 0) return -1;
    
//     if (out_code) {
//         *out_code = code;
//     } else {
//         write_bin(input_file, code, size);
//         free(code);
//     }
    
//     return size;
// }