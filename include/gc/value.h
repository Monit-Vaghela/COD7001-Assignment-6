#ifndef GC_VALUE_H
#define GC_VALUE_H

typedef struct HeapObj HeapObj;

typedef enum {
    VAL_INT,
    VAL_OBJ,
    VAL_NULL
} ValueType;

typedef struct {
    ValueType type;
    union {
        int int_value;
        HeapObj *obj;
    };
} Value;

#endif
