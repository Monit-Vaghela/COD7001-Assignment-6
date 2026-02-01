%{
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.tab.h"

extern int yylineno;
extern int lexical_error;

%}

%option noyywrap
%option yylineno
%option nounput

DIGIT [0-9]
LETTER [a-zA-Z_]
IDENTIFIER {LETTER}({LETTER}|{DIGIT})*
INTEGER {DIGIT}+
WHITESPACE [ \t\r\n]+

%%

"var" { yylval.lineno = yylineno; return VAR; }
"obj" { yylval.lineno = yylineno; return OBJ; }
"new" { yylval.lineno = yylineno; return NEW; }
"if" { yylval.lineno = yylineno; return IF; }
"else" { yylval.lineno = yylineno; return ELSE; }
"while" { yylval.lineno = yylineno; return WHILE; }

"set_left" { return SET_LEFT; }
"set_right" { return SET_RIGHT; }
"get_left" { return GET_LEFT; }
"get_right" { return GET_RIGHT; }
"remove_left" { return REMOVE_LEFT; }
"remove_right" { return REMOVE_RIGHT; }

"==" { return EQ; }
"!=" { return NE; }
"<=" { return LE; }
">=" { return GE; }

"=" { yylval.lineno = yylineno; return ASSIGN; }
"<" { return LT; }
">" { return GT; }
"+" { return PLUS; }
"-" { return MINUS; }
"*" { return MUL; }
"/" { return DIV; }

";" { return SEMI; }
"(" { return LPAREN; }
")" { return RPAREN; }
"{" { yylval.lineno = yylineno; return LBRACE; }
"}" { yylval.lineno = yylineno; return RBRACE; }
"." { return DOT; }

{INTEGER} {
    yylval.ival = atoi(yytext);
    return INTEGER;
}

{IDENTIFIER} {
    yylval.sval = strdup(yytext);
    return IDENTIFIER;
}

{WHITESPACE} { /* ignore */ }

"//".*\n { /* ignore single-line comments */ }

"/*"([^*]|(\*+[^*/]))*\*+"/" { /* ignore multi-line comments */ }

. {
    fprintf(stderr, "Lexical error at line %d: unknown character '%s'\n", yylineno, yytext);
}

%%