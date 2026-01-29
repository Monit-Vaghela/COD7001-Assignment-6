To run -

flex src/parser/lexer.l
bison -d src/parser/parser.y
gcc parser/*.c parser.tab.c lex.yy.c ir/*.c test_ir.c -o test_ir
./src/test_ir src/test.txt