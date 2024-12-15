%{
#include "Shell.h"

int yylex(void);
%}

%union {
   Expression * expr;
   ArgsList argsList;
   RedirectInfo redirect;
   char * singleArg;
}

%left ';'
%nonassoc '&'
%left AND OR
%left '|'
%left <redirect> REDIR

%token ERROR
%token <singleArg> SINGLE_ARG

%type <expr> expressionOrNothing
%type <expr> expression
%type <argsList> command

%%

commandLine:
   expressionOrNothing '\n' {
      shell.parsedExpr = $1;
      YYACCEPT;
   }
 | expressionOrNothing error '\n' {
      freeExpression($1);
      yyclearin;
      YYABORT;
   }
;

expressionOrNothing:
   {
      $$ = alloc((Expression){ ET_EMPTY });
   }
 | expression;

expression:
   command {
      $$ = alloc((Expression){ ET_SIMPLE, .argsList = $1 });
   }
 | expression ';' expression {
      $$ = alloc((Expression){ ET_SEQUENCE, $1, $3 });
   }
 | expression AND expression {
      $$ = alloc((Expression){ ET_SEQUENCE_AND, $1, $3 });
   }
 | expression OR expression {
      $$ = alloc((Expression){ ET_SEQUENCE_OR, $1, $3 });
   }
 | expression '|' expression {
      $$ = alloc((Expression){ ET_PIPE, $1, $3 });
   }
 | expression REDIR SINGLE_ARG {
      $2.fileName = $3;
      $$ = alloc((Expression){ ET_REDIRECT, $1, .redirect = $2 });
   }
 | expression '&' {
      $$ = alloc((Expression){ ET_BG, $1 });
   }
 | '(' expressionOrNothing ')' {
      $$ = $2;
   }
;

command:
   SINGLE_ARG {
      $$ = newArgsList();
      addArgToList(&$$, $1);
   }
 | command SINGLE_ARG {
      $$ = $1;
      addArgToList(&$$, $2);
   }
;

%%
