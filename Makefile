CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude -D_POSIX_C_SOURCE=200809L 
CFLAGS += -Wno-unused-function

# Source files
SRC := \
	src/debugger/intial.c \
	src/ir/ir.c \
	src/parser/ast.c \
	src/parser/parser_driver.c \
	src/parser/symbol_table.c \
	src/shell/builtins.c \
	src/shell/compiler.c \
	src/shell/executor.c \
	src/shell/parser.c \
	src/shell/processmanager.c \
	src/shell/shell.c \
	src/shell/tokenizer.c \
	src/shell/utils.c \
	src/VM/* \
	src/gc/* \
	src/main.c

# Generated files
LEX_C   = src/lex.yy.c
YACC_C  = src/parser.tab.c
YACC_H  = src/parser.tab.h

# Default target
all: myShell

myShell: $(LEX_C) $(YACC_C)
	$(CC) $(CFLAGS) $(SRC) $(LEX_C) $(YACC_C) -o myShell

# Flex
$(LEX_C): src/parser/lexer.l
	flex -o $(LEX_C) $<

# Bison
$(YACC_C): src/parser/parser.y
	bison -d -o $(YACC_C) $<

clean:
	rm -f myShell src/test_ir
	rm -f src/lex.yy.c src/parser.tab.c src/parser.tab.h
	find . -name "*.bin" -type f -delete
