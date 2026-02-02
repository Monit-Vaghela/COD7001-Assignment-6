#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser/ast.h"

static ast_node *ast_create_empty_node(ast_node_type type, int line) {
    /*
    Inputs : Type of ast node
    Output : returns a pointer to the new node created
    */

    // sizeof(ast_node) will be size of all of its
    // components combined, and that much uninitialized
    // memory will be allocated
    ast_node *node = malloc(sizeof(ast_node));
    if (!node)
    {
        perror("malloc");
        exit(1);
    }

    // Initialize everything to either
    // NULL or ZERO except type
    node->type = type;
    node->left = NULL;
    node->right = NULL;
    node->extra = NULL;
    node->int_value = 0;
    node->dbg.line = line;
    node->name = NULL;
    node->op = 0;

    return node;
}

ast_node *ast_make_int(int value, int line) {
    /*
    Input : The value of the integer
    Output : pointer to the new INT type AST node
    */
    ast_node *node = ast_create_empty_node(AST_INT, line);
    node->int_value = value;
    return node;
}

ast_node *ast_make_ident(char *name, int line) {
    /*
    Input : The name of the identifier
    Output : pointer to the new IDENT type AST node
    */
    ast_node *node = ast_create_empty_node(AST_IDENT, line);
    node->name = strdup(name);
    return node;
}

ast_node *ast_make_binop(char op, ast_node *left, ast_node *right, int line) {
    /*
    Inputs :
        1) Binary operator used in the expression
        2) left AST node
        3) right AST node
    Output : pointer to the new BINOP type AST node
    */
    ast_node *node = ast_create_empty_node(AST_BINOP, line);
    node->op = op;
    node->left = left;
    node->right = right;
    return node;
}

ast_node *ast_make_assign(char *name, ast_node *expr, int line) {
    /*
    Inputs :
        1) name of the variable being assigned
        2) expression assigned to the variable
    Output : pointer to the new ASSIGN type AST node
    */
    ast_node *node = ast_create_empty_node(AST_ASSIGN, line);
    node->name = strdup(name);

    // Assign has only one child — use left
    node->left = expr;
    return node;
}

ast_node *ast_make_var_decl(char *name, ast_node *expr, int line) {
    /*
    Inputs :
        1) variable name
        2) optional initializer expression
    Output : pointer to VAR_DECL AST node
    */
    ast_node *node = ast_create_empty_node(AST_VAR_DECL, line);
    node->name = strdup(name);

    /*
    Can be NULL
    Example:
        var x;
        var x = 10;
    */
    node->left = expr;
    return node;
}

ast_node *ast_make_obj_decl(char *name, ast_node *expr, int line) {
    /*
    Inputs :
        1) object name
        2) initializer expression
    Output : pointer to OBJ_DECL AST node
    */
    ast_node *node = ast_create_empty_node(AST_OBJ_DECL, line);
    node->name = strdup(name);
    node->left = expr;
    return node;
}

ast_node *ast_make_print(ast_node *expr, int line) {
    ast_node *node = ast_create_empty_node(AST_PRINT, line);
    node->left = expr;
    return node;
}

ast_node *ast_make_if(ast_node *cond, ast_node *then_block, int line) {
    /*
    Inputs :
        1) condition expression
        2) then block
    Output : pointer to IF AST node
    */
    ast_node *node = ast_create_empty_node(AST_IF, line);
    node->left = cond;
    node->right = then_block;
    return node;
}

ast_node *ast_make_if_else(ast_node *cond, ast_node *then_block,
                           ast_node *else_block, int line) {
    /*
    Inputs :
        1) condition expression
        2) then block
        3) else block
    Output : pointer to IF_ELSE AST node
    */
    ast_node *node = ast_create_empty_node(AST_IF_ELSE, line);
    node->left = cond;
    node->right = then_block;
    node->extra = else_block;
    return node;
}

ast_node *ast_make_while(ast_node *cond, ast_node *body, int line) {
    /*
    Inputs :
        1) loop condition
        2) loop body
    Output : pointer to WHILE AST node
    */
    ast_node *node = ast_create_empty_node(AST_WHILE, line);
    node->left = cond;
    node->right = body;
    return node;
}

ast_node *ast_make_block(ast_node *stmt_list, int line) {
    /*
    Input : list of statements
    Output : BLOCK AST node
    */
    ast_node *node = ast_create_empty_node(AST_BLOCK, line);
    node->left = stmt_list;
    return node;
}

ast_node *ast_make_new(int line) {
    /*
    Output : NEW AST node
    */
    return ast_create_empty_node(AST_NEW, line);
}

ast_node *ast_make_obj_set_left(char *obj_name, ast_node *value, int line) {
    /*
    Set left field of object
    */
    ast_node *node = ast_create_empty_node(AST_OBJ_SET_LEFT, line);
    node->name = strdup(obj_name);
    node->left = value;
    return node;
}

ast_node *ast_make_obj_set_right(char *obj_name, ast_node *value, int line) {
    /*
    Set right field of object
    */
    ast_node *node = ast_create_empty_node(AST_OBJ_SET_RIGHT, line);
    node->name = strdup(obj_name);
    node->left = value;
    return node;
}

ast_node *ast_make_obj_get_left(char *obj_name, int line) {
    /*
    Get left field of object
    */
    ast_node *node = ast_create_empty_node(AST_OBJ_GET_LEFT, line);
    node->name = strdup(obj_name);
    return node;
}

ast_node *ast_make_obj_get_right(char *obj_name, int line) {
    /*
    Get right field of object
    */
    ast_node *node = ast_create_empty_node(AST_OBJ_GET_RIGHT, line);
    node->name = strdup(obj_name);
    return node;
}

ast_node *ast_make_obj_remove_left(char *obj_name, int line) {
    /*
    Remove left field of object
    */
    ast_node *node = ast_create_empty_node(AST_OBJ_REMOVE_LEFT, line);
    node->name = strdup(obj_name);
    return node;
}

ast_node *ast_make_obj_remove_right(char *obj_name, int line) {
    /*
    Remove right field of object
    */
    ast_node *node = ast_create_empty_node(AST_OBJ_REMOVE_RIGHT, line);
    node->name = strdup(obj_name);
    return node;
}

ast_node *ast_append_statement(ast_node *list, ast_node *stmt)
{
    /*
    Append a statement at the end of statement list
    */
    if (!stmt)
        return list;

    if (!list)
        return stmt;

    ast_node *temp = list;

    // Traverse until last node
    while (temp->right != NULL)
        temp = temp->right;

    temp->right = stmt;
    return list;
}

static void print_dbg(ast_node *node)
{
    // Debug metadata printer
    printf(" [line=%d scope=%d var=%d ctrl=%d]",
           node->dbg.line,
           node->dbg.scope_id,
           node->dbg.var_id,
           node->dbg.control_id);
}

void ast_print(ast_node *node, int indent)
{
    /*
    Recursively prints AST structure
    */
    if (!node)
        return;

    for (int i = 0; i < indent; i++)
        printf("  ");

    switch (node->type)
    {
        case AST_INT:
            printf("INT(%d)", node->int_value);
            print_dbg(node);
            printf("\n");
            break;

        case AST_IDENT:
            printf("IDENT(%s)", node->name);
            print_dbg(node);
            printf("\n");
            break;

        case AST_BINOP:
            printf("BINOP(%c)", node->op);
            print_dbg(node);
            printf("\n");
            ast_print(node->left, indent + 1);
            ast_print(node->right, indent + 1);
            break;

        case AST_ASSIGN:
            printf("ASSIGN(%s)", node->name);
            print_dbg(node);
            printf("\n");
            ast_print(node->left, indent + 1);
            break;

        case AST_VAR_DECL:
            printf("VAR_DECL(%s)", node->name);
            print_dbg(node);
            printf("\n");
            if (node->left)
                ast_print(node->left, indent + 1);
            break;

        case AST_OBJ_DECL:
            printf("OBJ_DECL(%s)", node->name);
            print_dbg(node);
            printf("\n");
            if (node->left)
                ast_print(node->left, indent + 1);
            break;

        case AST_PRINT:
            printf("PRINT");
            print_dbg(node);
            printf("\n");
            ast_print(node->left, indent + 1);
            break;

        case AST_NEW:
            printf("NEW");
            print_dbg(node);
            printf("\n");
            break;

        case AST_OBJ_SET_LEFT:
            printf("OBJ_SET_LEFT(%s)", node->name);
            print_dbg(node);
            printf("\n");
            ast_print(node->left, indent + 1);
            break;

        case AST_OBJ_SET_RIGHT:
            printf("OBJ_SET_RIGHT(%s)", node->name);
            print_dbg(node);
            printf("\n");
            ast_print(node->left, indent + 1);
            break;

        case AST_OBJ_GET_LEFT:
            printf("OBJ_GET_LEFT(%s)", node->name);
            print_dbg(node);
            printf("\n");
            break;

        case AST_OBJ_GET_RIGHT:
            printf("OBJ_GET_RIGHT(%s)", node->name);
            print_dbg(node);
            printf("\n");
            break;

        case AST_OBJ_REMOVE_LEFT:
            printf("OBJ_REMOVE_LEFT(%s)", node->name);
            print_dbg(node);
            printf("\n");
            break;

        case AST_OBJ_REMOVE_RIGHT:
            printf("OBJ_REMOVE_RIGHT(%s)", node->name);
            print_dbg(node);
            printf("\n");
            break;

        case AST_IF:
            printf("IF");
            print_dbg(node);
            printf("\n");
            ast_print(node->left, indent + 1);
            ast_print(node->right, indent + 1);
            break;

        case AST_IF_ELSE:
            printf("IF_ELSE");
            print_dbg(node);
            printf("\n");
            ast_print(node->left, indent + 1);
            ast_print(node->right, indent + 1);
            ast_print(node->extra, indent + 1);
            break;

        case AST_WHILE:
            printf("WHILE");
            print_dbg(node);
            printf("\n");
            ast_print(node->left, indent + 1);
            ast_print(node->right, indent + 1);
            break;

        case AST_BLOCK:
            printf("BLOCK");
            print_dbg(node);
            printf("\n");
            ast_print(node->left, indent + 1);
            break;

        default:
            printf("UNKNOWN NODE\n");
    }


    /*
    Traverse linked statement list
    */
    if ((node->type == AST_VAR_DECL ||
         node->type == AST_OBJ_DECL ||
         node->type == AST_ASSIGN ||
         node->type == AST_PRINT ||
         node->type == AST_OBJ_SET_LEFT ||
         node->type == AST_OBJ_SET_RIGHT ||
         node->type == AST_OBJ_REMOVE_LEFT ||
         node->type == AST_OBJ_REMOVE_RIGHT ||
         node->type == AST_BLOCK) &&
        node->right)
    {
        ast_print(node->right, indent);
    }
}

void ast_free(ast_node *node)
{
    /*
    Recursively frees AST memory
    */
    if (!node)
        return;

    ast_free(node->left);
    ast_free(node->right);
    ast_free(node->extra);

    if (node->name)
        free(node->name);

    free(node);
}