#ifndef _SHELL_H
#define _SHELL_H

#include "Expression.h"

typedef struct ProgramArgs {
  int argc;
  char **argv;
} ProgramArgs;

typedef struct Shell {
  ProgramArgs args;
  Expression *parsedExpr;
  bool showExprTree;
} Shell;

int yyparse(void);
int yyparse_string(char *str);

void EndOfFile(void);

void yyerror(char const *str);

void commandExecution(int parsingResult);

extern Shell shell;
extern bool interactiveMode;
extern int shellStatus;

#endif
