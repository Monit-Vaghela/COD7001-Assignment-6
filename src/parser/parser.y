%{
#include <stdio.h>
#include <stdlib.h>
#include "parser/ast.h"
#include "parser/symbol_table.h"

struct ast_node;

int semantic_error = 0;
int parse_error = 0;

int control_counter = 0;
int alloc_counter = 0;
int var_counter = 0;

ast_node *ast_root = NULL;
extern int yylex(void);
extern int yylineno;

void yyerror(const char *s);

/* ---------- DEBUG METADATA INIT ---------- */
#define INIT_DBG(node)                             \
    do {                                           \
        (node)->dbg.line = yylineno;               \
        (node)->dbg.var_id = -1;                   \
        (node)->dbg.scope_id = get_current_scope();\
        (node)->dbg.control_id = -1;               \
        (node)->dbg.alloc_site_id = -1;            \
    } while (0)

%}

%define parse.error detailed

%union {
    int ival;
    char *sval;
    struct ast_node *node;
    int lineno;
}

/* Keywords - track their line numbers */
%token <lineno> VAR IF ELSE WHILE

/* Identifiers */
%token <sval> IDENTIFIER

/* Literals */
%token <ival> INTEGER

/* Operators */
%token PLUS MINUS MUL DIV
%token <lineno> ASSIGN
%token EQ NE LT GT LE GE

/* Syntax */
%token LPAREN RPAREN
%token <lineno> LBRACE RBRACE
%token SEMI

/* Non terminals */
%type <node> program
%type <node> stmt stmt_list block
%type <node> variable_decl assignment
%type <node> if_stmt while_stmt
%type <node> expression equality comparison term factor unary primary

%start program

%%

program:
    stmt_list
    {
        ast_root = ast_make_block($1, 1);
        ast_root->dbg.line = 1;
        ast_root->dbg.scope_id = 0;
        ast_root->dbg.var_id = -1;
        ast_root->dbg.control_id = -1;
        ast_root->dbg.alloc_site_id = -1;
    }
    ;

stmt_list:
    /* empty */
    {
        $$ = NULL;
    }
    | stmt_list stmt
    {
        $$ = ast_append_statement($1, $2);
    }
    ;

stmt:
    variable_decl { $$ = $1; }
    | assignment  { $$ = $1; }
    | if_stmt     { $$ = $1; }
    | while_stmt  { $$ = $1; }
    | block       { $$ = $1; }
    | SEMI        { $$ = NULL; }
    ;

block:
    LBRACE
    {
        enter_scope();
    }
    stmt_list
    RBRACE
    {
        exit_scope();
        $$ = ast_make_block($3, $1);  /* Use LBRACE line number */
        $$->dbg.line = $1;
        $$->dbg.scope_id = get_current_scope();
        $$->dbg.var_id = -1;
        $$->dbg.control_id = -1;
        $$->dbg.alloc_site_id = -1;
    }
    ;

/* ---------------- VARIABLE DECL ---------------- */

variable_decl:
    VAR IDENTIFIER SEMI
    {
        int var_id = -1;
        
        if (symtab_insert($2, 0) != 0) {
            fprintf(stderr,
                    "Error at line %d: redeclaration of variable '%s'\n",
                    $1, $2);
            semantic_error = 1;
        } else {
            var_id = ++var_counter;
        }

        $$ = ast_make_var_decl($2, NULL, $1);  /* Use VAR token line */
        $$->dbg.line = $1;
        $$->dbg.scope_id = get_current_scope();
        $$->dbg.var_id = var_id;
        $$->dbg.control_id = -1;
        $$->dbg.alloc_site_id = -1;
    }

    | VAR IDENTIFIER ASSIGN expression SEMI
    {
        int var_id = -1;
        
        if (symtab_insert($2, 0) != 0) {
            fprintf(stderr,
                    "Error at line %d: redeclaration of variable '%s'\n",
                    $1, $2);
            semantic_error = 1;
        } else {
            var_id = ++var_counter;
        }

        $$ = ast_make_var_decl($2, $4, $1);  /* Use VAR token line */
        $$->dbg.line = $1;
        $$->dbg.scope_id = get_current_scope();
        $$->dbg.var_id = var_id;
        $$->dbg.control_id = -1;
        $$->dbg.alloc_site_id = -1;
    }
    ;

/* ---------------- ASSIGNMENT ---------------- */

assignment:
    IDENTIFIER ASSIGN expression SEMI
    {
        if (symtab_update($1, 0) != 0) {
            fprintf(stderr,
                    "Error at line %d: assignment to undeclared variable '%s'\n",
                    $2, $1);
            semantic_error = 1;
        }

        $$ = ast_make_assign($1, $3, $2);  /* Use ASSIGN token line */
        $$->dbg.line = $2;
        $$->dbg.scope_id = get_current_scope();
        $$->dbg.var_id = -1;
        $$->dbg.control_id = -1;
        $$->dbg.alloc_site_id = -1;
    }
    ;

/* ---------------- IF / WHILE ---------------- */

if_stmt:
    IF LPAREN expression RPAREN stmt
    {
        $$ = ast_make_if($3, $5, $1);  /* Use IF token line */
        $$->dbg.line = $1;
        $$->dbg.scope_id = get_current_scope();
        $$->dbg.var_id = -1;
        $$->dbg.control_id = ++control_counter;
        $$->dbg.alloc_site_id = -1;
    }

    | IF LPAREN expression RPAREN stmt ELSE stmt
    {
        $$ = ast_make_if_else($3, $5, $7, $1);  /* Use IF token line */
        $$->dbg.line = $1;
        $$->dbg.scope_id = get_current_scope();
        $$->dbg.var_id = -1;
        $$->dbg.control_id = ++control_counter;
        $$->dbg.alloc_site_id = -1;
    }
    ;

while_stmt:
    WHILE LPAREN expression RPAREN stmt
    {
        $$ = ast_make_while($3, $5, $1);  /* Use WHILE token line */
        $$->dbg.line = $1;
        $$->dbg.scope_id = get_current_scope();
        $$->dbg.var_id = -1;
        $$->dbg.control_id = ++control_counter;
        $$->dbg.alloc_site_id = -1;
    }
    ;

/* ---------------- EXPRESSIONS ---------------- */

expression:
    equality { $$ = $1; }
    ;

equality:
    comparison { $$ = $1; }
    | equality EQ comparison
    {
        $$ = ast_make_binop('E', $1, $3, yylineno);
        INIT_DBG($$);
    }
    | equality NE comparison
    {
        $$ = ast_make_binop('N', $1, $3, yylineno);
        INIT_DBG($$);
    }
    ;

comparison:
    term { $$ = $1; }
    | comparison LT term
    {
        $$ = ast_make_binop('<', $1, $3, yylineno);
        INIT_DBG($$);
    }
    | comparison GT term
    {
        $$ = ast_make_binop('>', $1, $3, yylineno);
        INIT_DBG($$);
    }
    | comparison LE term
    {
        $$ = ast_make_binop('L', $1, $3, yylineno);
        INIT_DBG($$);
    }
    | comparison GE term
    {
        $$ = ast_make_binop('G', $1, $3, yylineno);
        INIT_DBG($$);
    }
    ;

term:
    factor { $$ = $1; }
    | term PLUS factor
    {
        $$ = ast_make_binop('+', $1, $3, yylineno);
        INIT_DBG($$);
    }
    | term MINUS factor
    {
        $$ = ast_make_binop('-', $1, $3, yylineno);
        INIT_DBG($$);
    }
    ;

factor:
    unary { $$ = $1; }
    | factor MUL unary
    {
        $$ = ast_make_binop('*', $1, $3, yylineno);
        INIT_DBG($$);
    }
    | factor DIV unary
    {
        $$ = ast_make_binop('/', $1, $3, yylineno);
        INIT_DBG($$);
    }
    ;

unary:
    PLUS unary { $$ = $2; }
    | MINUS unary
    {
        $$ = ast_make_binop('-', ast_make_int(0, yylineno), $2, yylineno);
        INIT_DBG($$);
    }
    | primary { $$ = $1; }
    ;

primary:
    INTEGER
    {
        $$ = ast_make_int($1, yylineno);
        INIT_DBG($$);
    }

    | IDENTIFIER
    {
        if (!lookup_symbol($1)) {
            fprintf(stderr,
                    "Error at line %d: variable '%s' not declared\n",
                    yylineno, $1);
            semantic_error = 1;
        }

        $$ = ast_make_ident($1, yylineno);
        INIT_DBG($$);
    }

    | LPAREN expression RPAREN
    {
        $$ = $2;
    }
    ;

%%

void yyerror(const char *s)
{
    parse_error = 1;
    fprintf(stderr, "Parse error at line %d: %s\n", yylineno, s);
}

void reset_parser_state(void)
{
    var_counter = 0;
    control_counter = 0;
    alloc_counter = 0;
    semantic_error = 0;
    parse_error = 0;
    ast_root = NULL;
}