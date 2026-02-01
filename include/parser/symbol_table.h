#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

typedef enum {
    SYM_VAR,
    SYM_OBJ
} symbol_type;

typedef struct symbol {
    char *name;
    int value;
    int scope;
    symbol_type type;
    struct symbol *next;
} symbol_t;

void symtab_init();
void symtab_cleanup();
void enter_scope();
void exit_scope();
int symtab_insert(const char *name, int value);
int symtab_insert_obj(const char *name);
int symtab_update(const char *name, int value);
void symtab_print();
symbol_t *lookup_symbol(const char *name);
symbol_t *get_symbol_table(void);
int get_current_scope(void);

#endif