#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "ir.h"
#include "../parser/ast.h"

static FILE *out;

/* variable → slot mapping */
typedef struct {
    char *name;
    int slot;
} var_entry;

static var_entry vars[128];
static int var_count = 0;


static int get_slot(const char *name)
{
    for(int i=0;i<var_count;i++)
    {
        if(strcmp(vars[i].name, name) == 0)
        {
            return vars[i].slot;
        }
    }
    vars[var_count].name = strdup(name);
    vars[var_count].slot = var_count;
    var_count++;

    return var_count-1;
}

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
            int slot = get_slot(node->name);
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
                default:
                    // for > < >= == 
                    break;
            }
            break;

        case AST_VAR_DECL: {
            int slot = get_slot(node->name);
            if (node->left) {
                gen(node->left);
                emit("STORE %d\n", slot);
            }
            break;
        }

        case AST_ASSIGN: {
            int slot = get_slot(node->name);
            gen(node->left);
            emit("STORE %d\n", slot);
            break;
        }
        case AST_BLOCK: {
            ast_node *stmt = node->left;
            while (stmt) {
                gen(stmt);
                stmt = stmt->right;
            }
            break;
        }

        default:
            // ignore for now
            break;
    }
}


void generate_ir(ast_node *root)
{
    out = fopen("out.asm", "w");
    if (!out) {
        perror("out.asm");
        exit(1);
    }

    gen(root);

    fprintf(out, "HALT\n");
    fclose(out);
}
