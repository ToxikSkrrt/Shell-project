#include "Display.h"

#include <stdio.h>

#include <unistd.h>

static void indentEmpty(int indentLen, int barPos) {
  for (int crtI = 1; crtI <= indentLen + barPos; ++crtI) {
    putchar(crtI % barPos == 0 ? '|' : ' ');
  }
}

static void indent(int indentLen, int barPos) {
  for (int crtI = 1; crtI < indentLen; ++crtI) {
    putchar(crtI % barPos == 0 ? '|' : ' ');
  }
  putchar('+');
  for (int crtI = 2; crtI < barPos; ++crtI) {
    if (indentLen % barPos == 0) {
      putchar('-');
    }
  }
  putchar('>');
}

static void printExprExt(Expression const *expr, int indentLen, int barPos) {
  static char const *exprTypeText[] =
      {
          "EMPTY",       "SIMPLE", "SEQUENCE", "SEQUENCE_AND",
          "SEQUENCE_OR", "BG",     "PIPE",     "REDIRECT",
      },
                    *redirTypeText[] = {"IN", "OUT", "APP"};

  if (expr == NULL) {
    return;
  }

  switch (expr->type) {
  case ET_EMPTY:
    indent(indentLen, barPos);
    printf("%s\n", exprTypeText[expr->type]);
    break;
  case ET_SIMPLE:
    indent(indentLen, barPos);
    printf("%s ", exprTypeText[expr->type]);
    for (int argI = 0; argI < expr->argc; ++argI) {
      printf("[%s]", expr->argv[argI]);
    }
    putchar('\n');
    break;
  case ET_REDIRECT:
    indent(indentLen, barPos);
    printf("%s %s ", exprTypeText[expr->type],
           redirTypeText[expr->redirect.type]);
    printf(expr->redirect.fd < 0 ? "[&]" : "[%d]", expr->redirect.fd);
    printf(" %s [%s]\n", expr->redirect.toOtherFd ? "fd" : "file",
           expr->redirect.fileName);
    printExprExt(expr->left, indentLen + barPos, barPos);
    break;
  case ET_BG:
    indent(indentLen, barPos);
    printf("%s\n", exprTypeText[expr->type]);
    printExprExt(expr->left, indentLen + barPos, barPos);
    break;
  default:
    indent(indentLen, barPos);
    printf("%s\n", exprTypeText[expr->type]);
    printExprExt(expr->left, indentLen + barPos, barPos);
    indentEmpty(indentLen, barPos);
    putchar('\n');
    printExprExt(expr->right, indentLen + barPos, barPos);
  }
}

void printExpr(Expression const *expr) { printExprExt(expr, 4, 4); }
