#include "Shell.h"
#include "Display.h"
#include "Evaluation.h"

#include <stdio.h>

#include <unistd.h>

#include <readline/history.h>
#include <readline/readline.h>

Shell shell;
bool interactiveMode;
int shellStatus = 0;

// Fonction appelée lorsque l'utilisateur tape Ctrl+D
void EndOfFile(void) {
  printf("\n");
  exit(shellStatus);
}

// Appelée par yyparse() sur erreur syntaxique
void yyerror(char const *str) { fprintf(stderr, "%s\n", str); }

/*
 * - Lecture de la ligne de commande à l'aide de readline() en mode interactif
 * - Mémorisation dans l'historique des commandes
 * - Analyse de la ligne lue
 */
int parseLine(void) {
  if (interactiveMode) {
    char prompt[64];
    snprintf(prompt, sizeof(prompt),
             "\33[1mmini_shell(\33[3%dm%d\33[0;1m):\33[0m ",
             shellStatus == EXIT_SUCCESS ? 2 /* vert */ : 1 /* rouge */,
             shellStatus);
    char *line = readline(prompt);
    if (line != NULL) {
      int ret;
      size_t len = strlen(line);
      if (len > 0) {
        add_history(line); // Enregistre la ligne non vide dans l'historique
      }
      line = realloc(line, len + 2);
      strcpy(line + len, "\n"); // Ajoute \n à la ligne pour qu'elle puisse être
                                // traitée par l'analyseur
      ret = yyparse_string(line);
      free(line);
      return ret;
    } else {
      EndOfFile();
      return -1;
    }
  } else {
    return yyparse();
  }
}

void commandExecution(int parsingResult) {
  if (parsingResult == 0) { // L'analyse a abouti
    if (shell.showExprTree) {
      printExpr(shell.parsedExpr);
    }
    shellStatus = evaluateExpr(shell.parsedExpr);
    freeExpression(shell.parsedExpr);
  } else { // L'analyse de la ligne de commande a donné une erreur
    shellStatus = 2;
  }
}


     /*--------------------------------------------------------------------------------------*\
      | Lorsque l'analyse de la ligne de commande est effectuée sans erreur, la variable     |
      | globale parsedExpr pointe sur un arbre représentant l'expression. Le type            |
      | « Expression » des nœuds de l'arbre est décrit dans le fichier Expression.h.         |
      |                                                                                      |
      | e étant de type Expression * :                                                       |
      | - e->type est un type d'expression, contenant une valeur définie par énumération     |
      |    dans Shell.h.                                                                     |
      | - e->left et e->right, de type Expression *, représentent une sous-expression gauche |
      |    et une sous-expression droite.                                                    |
      |    - Ces deux champs ne sont pas utilisés pour les types EMPTY et SIMPLE.            |
      |    - Pour les types SEQUENCE, SEQUENCE_AND, SEQUENCE_OR et PIPE, ces deux champs     |
      |       sont utilisés simultanément.                                                   |
      |    - Pour les autres types, seule l'expression gauche est utilisée.                  |
      | - Pour le type SIMPLE : e->argc (int) et e->argv (char * *) sont disponibles.        |
      | - Pour le type REDIRECT : e->redirect contient les informations de redirection.      |
      |    - e->redirect.type indique le type.                                               |
      |    - e->redirect.fd est le descripteur de fichier à rediriger.                       |
      |    - e->redirect.toOtherFd vaut vrai si la redirection est à faire vers un autre     |
      |       descripteur de fichier.                                                        |
      |    - e->redirect.fileName, de type char *, contient le nom du fichier (ou numéro de  |
      |       descripteur de fichier) vers lequel rediriger.                                 |
     \*--------------------------------------------------------------------------------------*/

int main(int argc, char *argv[]) {
  shell.args = (ProgramArgs){argc, argv};
  interactiveMode = isatty(STDIN_FILENO);
  if (interactiveMode) {
    using_history();
  }
  shell.showExprTree = interactiveMode;

  while (true) {
    commandExecution(parseLine());
  }

  printf("\n"); // Comme dans EndOfFile()
  return shellStatus;
}
