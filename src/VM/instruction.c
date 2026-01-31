#include <VM/instruction.h>
char *opcode_to_string(unsigned char op) {
    switch (op) {
        case PUSH: return "PUSH";
        case POP: return "POP";
        case DUP: return "DUP";

        case ADD: return "ADD";
        case SUB: return "SUB";
        case MUL: return "MUL";
        case DIV: return "DIV";
        case LT:  return "LT";
        case LTE: return "LTE";
        case GT:  return "GT";
        case GTE: return "GTE";
        case EQ:  return "EQ";

        case JMP: return "JMP";
        case JZ:  return "JZ";
        case JNZ: return "JNZ";

        case STORE: return "STORE";
        case LOAD:  return "LOAD";

        case CALL: return "CALL";
        case RET:  return "RET";

        case NEW_PAIR:     return "NEW_PAIR";
        case SET_LEFT:     return "SET_LEFT";
        case SET_RIGHT:    return "SET_RIGHT";
        case GET_LEFT:     return "GET_LEFT";
        case GET_RIGHT:    return "GET_RIGHT";
        case REMOVE_LEFT:  return "REMOVE_LEFT";
        case REMOVE_RIGHT: return "REMOVE_RIGHT";

        case MAKE_FUNC:     return "MAKE_FUNC";
        case MAKE_CLOSURE:  return "MAKE_CLOSURE";
        case CALL_CLOSURE:  return "CALL_CLOSURE";

        case GC: return "GC";

        case BREAK: return "BREAK";
        case HALT:  return "HALT";

        default: return "UNKNOWN";
    }
}