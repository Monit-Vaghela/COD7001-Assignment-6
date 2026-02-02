#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "ir/ir.h"
#include "parser/ast.h"

static char *ir_buf;
static int ir_size;
static int ir_cap;

static int label_id = 0;
static int ir_error = 0;  

static int *line_numbers = NULL;
static int line_count = 0;
static int line_cap = 0;
static int last_line = -1;

static ast_node **ast_roots = NULL;
static ast_node *current_ast_root = NULL;

static int new_label() {
    return label_id++;
}

char *get_ir_buf(){
    return ir_buf;
}

int *get_line_numbers() {
    return line_numbers;
}

int get_instruction_count() {
    return line_count;
}

ast_node **get_ast_roots() {
    return ast_roots;
}

int get_last_line() {
    return last_line;
}

static void ir_init(void)
{
    ir_cap = 4096;
    ir_size = 0;
    ir_buf = malloc(ir_cap);
    ir_buf[0] = '\0';
    
    line_cap = 256;
    line_count = 0;
    line_numbers = malloc(line_cap * sizeof(int));
    ast_roots = malloc(line_cap * sizeof(ast_node *));
}

typedef struct {
    char *name;
    int slot;
    int scope;
} var_entry;

static var_entry vars[128];
static int var_count = 0;

static int current_scope = 0;
static int next_slot = 0;

static int lookup_slot(const char *name)
{
    for(int i = var_count - 1; i >= 0; i--)
    {
        if(strcmp(vars[i].name, name) == 0)
        {
            return vars[i].slot;
        }
    }
    fprintf(stderr, "IR error: variable '%s' not found\n", name);
    ir_error = 1; 
    return 0;     
}

static int create_slot(const char *name)
{
    vars[var_count].name = strdup(name);
    vars[var_count].slot = next_slot++;
    vars[var_count].scope = current_scope;
    var_count++;
    
    return vars[var_count-1].slot;
}

static void emit(const char *fmt, ...)
{
    char temp[256];
    va_list args;

    va_start(args, fmt);
    vsnprintf(temp, sizeof(temp), fmt, args);
    va_end(args);

    int len = strlen(temp);

    if (ir_size + len + 1 >= ir_cap) {
        ir_cap *= 2;
        ir_buf = realloc(ir_buf, ir_cap);
    }

    memcpy(ir_buf + ir_size, temp, len);
    ir_size += len;
    ir_buf[ir_size] = '\0';
}

static void add_line(int line)
{
    if (line_count >= line_cap) {
        line_cap *= 2;
        line_numbers = realloc(line_numbers, line_cap * sizeof(int));
        ast_roots = realloc(ast_roots, line_cap * sizeof(ast_node *));
    }
    line_numbers[line_count] = line;
    ast_roots[line_count] = current_ast_root;
    line_count++;
}

static void emit_tracked(int line, const char *fmt, ...)
{
    char temp[256];
    va_list args;

    va_start(args, fmt);
    vsnprintf(temp, sizeof(temp), fmt, args);
    va_end(args);

    int len = strlen(temp);

    if (ir_size + len + 1 >= ir_cap) {
        ir_cap *= 2;
        ir_buf = realloc(ir_buf, ir_cap);
    }

    memcpy(ir_buf + ir_size, temp, len);
    ir_size += len;
    ir_buf[ir_size] = '\0';
    
    add_line(line);
}


static void gen(ast_node *node)
{
    if (!node) return;

    int line = node->dbg.line;
    if(line > last_line) last_line = line;
    switch (node->type)
    {
        case AST_INT:
            emit_tracked(line, "PUSH %d\n", node->int_value);
            break;

        case AST_IDENT: {
            int slot = lookup_slot(node->name);
            if (ir_error) return;  
            emit_tracked(line, "LOAD %d\n", slot);
            break;
        }
        
        case AST_BINOP:
            gen(node->left);
            if (ir_error) return; 
            gen(node->right);
            if (ir_error) return; 
            
            switch (node->op) {
                case '+': emit_tracked(line, "ADD\n"); break;
                case '-': emit_tracked(line, "SUB\n"); break;
                case '*': emit_tracked(line, "MUL\n"); break;
                case '/': emit_tracked(line, "DIV\n"); break;
            
                case '<': emit_tracked(line, "LT\n"); break;
                case '>': emit_tracked(line, "GT\n"); break;
                case 'L': emit_tracked(line, "LE\n"); break;
                case 'G': emit_tracked(line, "GE\n"); break;
                case 'E': emit_tracked(line, "EQ\n"); break;
            }
            break;
        
        case AST_WHILE : {
            int start = new_label();
            int end = new_label();

            emit("L%d:\n", start);

            gen(node->left);
            if (ir_error) return;  
            emit_tracked(line, "JZ L%d\n", end);

            gen(node->right);
            if (ir_error) return;  
            emit_tracked(line, "JMP L%d\n", start);

            emit("L%d:\n", end);
            
            break;
        }

        case AST_VAR_DECL: {
            int slot = create_slot(node->name);
            if (node->left) {
                gen(node->left);
                if (ir_error) return; 
                emit_tracked(line, "STORE %d\n", slot);
            }
            else {
                emit_tracked(line, "PUSH 0\n");
                emit_tracked(line, "STORE %d\n", slot);
            }
            break;
        }

        case AST_OBJ_DECL: {
            int slot = create_slot(node->name);
            if (node->left) {
                gen(node->left);
                if (ir_error) return;
                emit_tracked(line, "STORE %d\n", slot);
            }
            break;
        }

        case AST_NEW: {
            emit_tracked(line, "NEW_PAIR\n");
            break;
        }

        case AST_OBJ_SET_LEFT: {
            int slot = lookup_slot(node->name);
            if (ir_error) return;
            emit_tracked(line, "LOAD %d\n", slot);
            gen(node->left);
            if (ir_error) return;
            emit_tracked(line, "SET_LEFT\n");
            emit_tracked(line, "STORE %d\n", slot);
            break;
        }

        case AST_OBJ_SET_RIGHT: {
            int slot = lookup_slot(node->name);
            if (ir_error) return;
            emit_tracked(line, "LOAD %d\n", slot);
            gen(node->left);
            if (ir_error) return;
            emit_tracked(line, "SET_RIGHT\n");
            emit_tracked(line, "STORE %d\n", slot);
            break;
        }

        case AST_OBJ_GET_LEFT: {
            int slot = lookup_slot(node->name);
            if (ir_error) return;
            emit_tracked(line, "LOAD %d\n", slot);
            emit_tracked(line, "GET_LEFT\n");
            break;
        }

        case AST_OBJ_GET_RIGHT: {
            int slot = lookup_slot(node->name);
            if (ir_error) return;
            emit_tracked(line, "LOAD %d\n", slot);
            emit_tracked(line, "GET_RIGHT\n");
            break;
        }

        case AST_OBJ_REMOVE_LEFT: {
            int slot = lookup_slot(node->name);
            if (ir_error) return;
            emit_tracked(line, "LOAD %d\n", slot);
            emit_tracked(line, "REMOVE_LEFT\n");
            emit_tracked(line, "STORE %d\n", slot);
            break;
        }

        case AST_OBJ_REMOVE_RIGHT: {
            int slot = lookup_slot(node->name);
            if (ir_error) return;
            emit_tracked(line, "LOAD %d\n", slot);
            emit_tracked(line, "REMOVE_RIGHT\n");
            emit_tracked(line, "STORE %d\n", slot);
            break;
        }

        case AST_ASSIGN: {
            int slot = lookup_slot(node->name);
            if (ir_error) return;  
            gen(node->left);
            if (ir_error) return;  
            emit_tracked(line, "STORE %d\n", slot);
            break;
        }

        case AST_PRINT: {
            gen(node->left);
            if (ir_error) return;
            emit_tracked(line, "PRINT\n");
            break;
        }

        case AST_IF: {
            int end = new_label();

            gen(node->left);
            if (ir_error) return; 
            emit_tracked(line, "JZ L%d\n", end);

            gen(node->right);
            if (ir_error) return; 
            emit("L%d:\n", end);
            break;
        }

        case AST_IF_ELSE: {
            int else_lbl = new_label();
            int end_lbl  = new_label();

            gen(node->left);
            if (ir_error) return; 
            emit_tracked(line, "JZ L%d\n", else_lbl);

            gen(node->right);
            if (ir_error) return; 
            emit_tracked(line, "JMP L%d\n", end_lbl);

            emit("L%d:\n", else_lbl);
            gen(node->extra);
            if (ir_error) return; 

            emit("L%d:\n", end_lbl);
            break;
        }

        case AST_BLOCK: {
            current_scope++;

            ast_node *stmt = node->left;
            while (stmt) {
                gen(stmt);
                if (ir_error) {
                    current_scope--;
                    return;
                }
                if (stmt->type == AST_IF ||
                    stmt->type == AST_IF_ELSE ||
                    stmt->type == AST_WHILE ||
                    stmt->type == AST_BLOCK) {
                    break;
                }
                stmt = stmt->right;
            }

            while (var_count > 0 &&
                   vars[var_count - 1].scope == current_scope)
            {
                var_count--;
            }

            current_scope--;

            break;
        }

        default:
            break;
    }
}


char *generate_ir(ast_node *root)
{
    if (!root) return NULL;
    
    ir_init();
    ir_error = 0;
    current_ast_root = root;
    gen(root);
    
    if (ir_error) { 
        fprintf(stderr, "IR generation failed due to errors.\n");
        return NULL;
    }
    last_line++;
    emit_tracked(last_line , "HALT\n");
    return ir_buf;
}

void reset_ir_state(void)
{
    label_id = 0;
    next_slot = 0;
    var_count = 0;
    current_scope = 0;
    ir_error = 0;
    last_line = -1;
    current_ast_root = NULL;
    
    if (ir_buf) {
        free(ir_buf);
        ir_buf = NULL;
    }
    if (line_numbers) {
        free(line_numbers);
        line_numbers = NULL;
    }
    if (ast_roots) {
        free(ast_roots);
        ast_roots = NULL;
    }
    
    ir_size = 0;
    ir_cap = 0;
    line_count = 0;
    line_cap = 0;
    
    for (int i = 0; i < var_count; i++) {
        if (vars[i].name) {
            free(vars[i].name);
        }
    }
}