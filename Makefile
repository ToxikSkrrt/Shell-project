LEX = flex
YACC = yacc -d -v

SRC := $(wildcard *.c) y.tab.c lex.yy.c
OBJ := $(SRC:%.c=%.o)

CFLAGS = -std=c11 -Og -g -Wall -Wwrite-strings -Wcast-qual
CPPFLAGS = -D_XOPEN_SOURCE=700

OS_NAME := $(shell uname -s | tr a-z A-Z)
ifeq ($(OS_NAME), DARWIN)
LDLIBS = -lreadline -ly -ll
else
LDLIBS = -lreadline -ly -lfl
endif

Shell: $(OBJ)

Display.o: Display.c Display.h Expression.h
Evaluation.o: Evaluation.c Evaluation.h Expression.h Shell.h
Expression.o: Expression.c Expression.h
InternalCommands.o: InternalCommands.c InternalCommands.h Shell.h Expression.h
Shell.o: Shell.c Shell.h Expression.h Display.h Evaluation.h

y.tab.c y.tab.h: CmdParsing.y
	$(YACC) CmdParsing.y
lex.yy.c: CmdParsing.l y.tab.h
	$(LEX) CmdParsing.l

.PHONY: clean
clean:
	rm -f *.o y.tab.* y.output lex.yy.* Shell