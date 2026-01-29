
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "ir/ir.h"
#include "parser/ast.h"

static FILE *out;

static int label_id = 0;

static int new_label() {
    return label_id++;
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
    va_list args;
    va_start(args, fmt);
    vfprintf(out, fmt, args);
    va_end(args);
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
            
                case '<': emit("CMP 0\n"); break;
                case '>': emit("CMP 1\n"); break;
                case 'L': emit("CMP 2\n"); break;
                case 'G': emit("CMP 3\n"); break;
                case 'E': emit("CMP 4\n"); break;
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
}


void generate_ir(ast_node *root)
{
    out = fopen("log/out.asm", "w");
    if (!out) {
        perror("log/out.asm");
        exit(1);
    }

    gen(root);

    fprintf(out, "HALT\n");
    fclose(out);
}

