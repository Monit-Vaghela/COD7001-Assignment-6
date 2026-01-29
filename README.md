To run -

flex parser/lexer.l
bison -d parser/parser.y
gcc parser/*.c parser.tab.c lex.yy.c ir/*.c test_ir.c -o test_ir
./test_ir test.txt