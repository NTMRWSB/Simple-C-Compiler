LEX = flex
YACC = bison
CC = gcc
NASM = nasm
PP = g++
file = test
ALL: $(file)
lex.yy.c: lex.l
	$(LEX) lex.l

yacc.tab.c yacc.tab.h: yacc.y hashMap.h tree.h
	$(YACC) -d yacc.y

compiler: yacc.tab.c yacc.tab.h lex.yy.c hashMap.h tree.h tree.c
	$(CC) -o compiler yacc.tab.c lex.yy.c tree.c stack.c hashMap.c inner.c -lfl

Innercode: compiler
	./compiler $(file).c

assembly.asm: assembly.cpp Innercode
	$(PP) $< -o assembly.out && ./assembly.out

test.o: assembly.asm
	$(NASM) -f elf64 $< -o $@

test: test.o
	$(CC) -no-pie -o $@ $< 

clean:
	-rm -rf lex.yy.c yacc.tab.c yacc.tab.h compiler Grammatical Lexical Innercode assembly assembly.asm test.o test assembly.out
	