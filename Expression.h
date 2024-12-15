#ifndef _EXPRESSION_H
#define _EXPRESSION_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef enum expr_t {
  ET_EMPTY,        // Commande vide
  ET_SIMPLE,       // Commande simple
  ET_SEQUENCE,     // Séquence (;)
  ET_SEQUENCE_AND, // Séquence conditionnelle (&&)
  ET_SEQUENCE_OR,  // Séquence conditionnelle (||)
  ET_BG,           // Tâche en arrière-plan (&)
  ET_PIPE,         // Tube (|)
  ET_REDIRECT,     // Redirection
} expr_t;

typedef enum redirection_t {
  REDIR_IN,  // Entrée (< ou <&)
  REDIR_OUT, // Sortie (> ou >&)
  REDIR_APP, // Sortie, mode ajout (>>)
} redirection_t;

typedef struct ArgsList {
  char **args;
  int len, allocatedLen;
} ArgsList;

typedef struct RedirectInfo {
  redirection_t type;
  int fd;         // -1 si &> ou &>>
  bool toOtherFd; // si <& ou >&
  char *fileName;
} RedirectInfo;

typedef struct Expression Expression;
struct Expression {
  expr_t type;
  Expression *left;
  Expression *right;
  union {
    struct {
      char **argv;
      int argc;
    };
    ArgsList argsList;
    RedirectInfo redirect;
  };
};

#define alloc(...)                                                             \
  memcpy(malloc(sizeof(__VA_ARGS__)), &__VA_ARGS__, sizeof(__VA_ARGS__))

void freeExpression(Expression *expr);

ArgsList newArgsList(void);
void addArgToList(ArgsList *argsList, char *str);
void freeArgsList(ArgsList argsList);

#endif
