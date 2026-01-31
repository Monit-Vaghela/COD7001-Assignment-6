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

/* Line number tracking */
static int *line_numbers = NULL;
static int line_count = 0;
static int line_cap = 0;

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

static void ir_init(void)
{
    ir_cap = 4096;
    ir_size = 0;
    ir_buf = malloc(ir_cap);
    ir_buf[0] = '\0';
    
    /* Initialize line number array */
    line_cap = 256;
    line_count = 0;
    line_numbers = malloc(line_cap * sizeof(int));
}

// variable → slot mapping 
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
    printf("IR error: variable '%s' not found\n", name);
    exit(1);
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

/* Add line number to array */
static void add_line(int line)
{
    if (line_count >= line_cap) {
        line_cap *= 2;
        line_numbers = realloc(line_numbers, line_cap * sizeof(int));
    }
    line_numbers[line_count++] = line;
}

/* Emit instruction and track line number */
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
    
    /* Add line number to array */
    add_line(line);
}


static void gen(ast_node *node)
{
    if (!node) return;

    int line = node->dbg.line;

    switch (node->type)
    {
        case AST_INT:
            emit_tracked(line, "PUSH %d\n", node->int_value);
            break;

        case AST_IDENT: {
            int slot = lookup_slot(node->name);
            emit_tracked(line, "LOAD %d\n", slot);
            break;
        }
        
        case AST_BINOP:
            gen(node->left);
            gen(node->right);   
            
            switch (node->op) {
                case '+': emit_tracked(line, "ADD\n"); break;
                case '-': emit_tracked(line, "SUB\n"); break;
                case '*': emit_tracked(line, "MUL\n"); break;
                case '/': emit_tracked(line, "DIV\n"); break;
            
                case '<': emit_tracked(line, "LT\n"); break;
                case '>': emit_tracked(line, "GT\n"); break;
                case 'L': emit_tracked(line, "CMP 2\n"); break;
                case 'G': emit_tracked(line, "CMP 3\n"); break;
                case 'E': emit_tracked(line, "EQ\n"); break;
                case 'N': emit_tracked(line, "CMP 5\n"); break;
            }
            break;
        
        case AST_WHILE : {
            int start = new_label();
            int end = new_label();

            emit("L%d:\n", start);

            gen(node->left);
            emit_tracked(line, "JZ L%d\n", end);

            gen(node->right);
            emit_tracked(line, "JMP L%d\n", start);

            emit("L%d:\n", end);
            
            break;
        }

        case AST_VAR_DECL: {
            int slot = create_slot(node->name);
            if (node->left) {
                gen(node->left);
                emit_tracked(line, "STORE %d\n", slot);
            }
            else {
                emit_tracked(line, "PUSH 0\n");
                emit_tracked(line, "STORE %d\n", slot);
            }
            break;
        }

        case AST_ASSIGN: {
            int slot = lookup_slot(node->name);
            gen(node->left);
            emit_tracked(line, "STORE %d\n", slot);
            break;
        }

        case AST_IF: {
            int end = new_label();

            gen(node->left);
            emit_tracked(line, "JZ L%d\n", end);

            gen(node->right);
            emit("L%d:\n", end);
            break;
        }

        case AST_IF_ELSE: {
            int else_lbl = new_label();
            int end_lbl  = new_label();

            gen(node->left);
            emit_tracked(line, "JZ L%d\n", else_lbl);

            gen(node->right);
            emit_tracked(line, "JMP L%d\n", end_lbl);

            emit("L%d:\n", else_lbl);
            gen(node->extra);

            emit("L%d:\n", end_lbl);
            break;
        }

        case AST_BLOCK: {
            current_scope++;

            ast_node *stmt = node->left;
            while (stmt) {
                gen(stmt);
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
    ir_init();
    gen(root);
    emit("HALT\n");
    return ir_buf;
}

void reset_ir_state(void)
{
    label_id = 0;
    next_slot = 0;
    var_count = 0;
    current_scope = 0;
    
    // Free and reset buffers
    if (ir_buf) {
        free(ir_buf);
        ir_buf = NULL;
    }
    if (line_numbers) {
        free(line_numbers);
        line_numbers = NULL;
    }
    
    ir_size = 0;
    ir_cap = 0;
    line_count = 0;
    line_cap = 0;
    
    // Free variable names
    for (int i = 0; i < var_count; i++) {
        if (vars[i].name) {
            free(vars[i].name);
        }
    }
}