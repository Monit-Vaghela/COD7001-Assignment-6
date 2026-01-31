
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "ir/ir.h"
#include "parser/ast.h"

static char *ir_buf;
static int ir_size;
static int ir_cap;

static int debug_info[10000];
static int debug_info_pointer = 0;
static int label_id = 0;

static int new_label() {
    return label_id++;
}

char *get_ir_buf(){
    return ir_buf;
}
int *get_debug_info(){
    return debug_info;
}
int get_debug_size(){
    return debug_info_pointer;
}

static void ir_init(void)
{
    ir_cap = 4096;
    ir_size = 0;
    ir_buf = malloc(ir_cap);
    ir_buf[0] = '\0';
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
    for(int i = var_count - 1;i>=0;i--)
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




// static int get_slot(const char *name)
// {
//     for(int i=0;i<var_count;i++)
//     {
//         if(strcmp(vars[i].name, name) == 0)
//         {
//             return vars[i].slot;
//         }
//     }
//     vars[var_count].name = strdup(name);
//     vars[var_count].slot = var_count;
//     var_count++;

//     return var_count-1;
// }

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


static void gen(ast_node *node)
{
    if (!node) return;

    switch (node->type)
    {
        case AST_INT:
            emit("PUSH %d\n", node->int_value);
            break;

        case AST_IDENT: {
            int slot = lookup_slot(node->name);
            emit("LOAD %d\n", slot);
            break;
        }
        case AST_BINOP:
            gen(node->left);
            gen(node->right);   
            
            switch (node->op) {

                case '+': emit("ADD\n"); break;
                case '-': emit("SUB\n"); break;
                case '*': emit("MUL\n"); break;
                case '/': emit("DIV\n"); break;
            
                case '<': emit("LT\n"); break;
                case '>': emit("GT\n"); break;
                case 'L': emit("CMP 2\n"); break;
                case 'G': emit("CMP 3\n"); break;
                case 'E': emit("EQ\n"); break;
                case 'N': emit("CMP 5\n"); break;
            }
            break;
        
        case AST_WHILE : {

            int start = new_label();
            int end = new_label();

            emit("L%d:\n", start);

            gen(node->left);
            emit("JZ L%d\n", end);

            gen(node->right);
            emit("JMP L%d\n", start);

            emit("L%d:\n", end);
            
            break;
        }

        case AST_VAR_DECL: {
            int slot = create_slot(node->name);
            if (node->left) {
                gen(node->left);
                emit("STORE %d\n", slot);
            }
            break;
        }

        case AST_ASSIGN: {
            int slot = lookup_slot(node->name);
            gen(node->left);
            emit("STORE %d\n", slot);
            break;
        }

        case AST_IF: {
            int end = new_label();

            gen(node->left);          // condition
            emit("JZ L%d\n", end);

            gen(node->right);         // then block
            emit("L%d:\n", end);
            break;
        }

        case AST_IF_ELSE: {
            int else_lbl = new_label();
            int end_lbl  = new_label();

            gen(node->left);              // condition
            emit("JZ L%d\n", else_lbl);

            gen(node->right);             // then
            emit("JMP L%d\n", end_lbl);

            emit("L%d:\n", else_lbl);
            gen(node->extra);             // else

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
            // ignore for now
            break;
    }
    debug_info[debug_info_pointer++] = node->lineno;
}


int generate_ir(ast_node *root)
{
    ir_init();
    gen(root);
    emit("HALT\n");
    return 0;
    
    // out = fopen("log/out.asm", "w");
    // if (!out) {
    //     perror("log/out.asm");
    //     exit(1);
    // }

    // gen(root);

    // fprintf(out, "HALT\n");
    // fclose(out);
}