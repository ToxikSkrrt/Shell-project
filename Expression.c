#include "Expression.h"

void freeExpression(Expression *expr) {
  if (expr == NULL) {
    return;
  }

  freeExpression(expr->left);
  freeExpression(expr->right);

  if (expr->type == ET_SIMPLE) {
    freeArgsList(expr->argsList);
  } else if (expr->type == ET_REDIRECT) {
    free(expr->redirect.fileName);
  }

  free(expr);
}

ArgsList newArgsList(void) {
  ArgsList argsList = {NULL, 0, 4};
  argsList.args =
      (char **)calloc(argsList.allocatedLen, sizeof(*argsList.args));
  return argsList;
}

void addArgToList(ArgsList *argsList, char *str) {
  if (argsList->len + 1 >= argsList->allocatedLen) {
    argsList->allocatedLen *= 2;
    char **newArgs = (char **)calloc(argsList->allocatedLen, sizeof(*newArgs));
    memcpy(newArgs, argsList->args, argsList->len * sizeof(*argsList->args));
    argsList->args = newArgs;
  }
  argsList->args[argsList->len] = str;
  ++argsList->len;
}

void freeArgsList(ArgsList argsList) {
  for (int i = 0; i < argsList.len; ++i) {
    free(argsList.args[i]);
  }
  free(argsList.args);
}
