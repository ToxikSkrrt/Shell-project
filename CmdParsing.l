%{
#include "Shell.h"
#include "y.tab.h"

#include <ctype.h>

#define YY_NO_UNPUT
#define YY_NO_INPUT
%}

COMMENT (#.*)
ARG_NOQUOTES  (([^\n\t `"'<>&|;()\\]|\\.)+)
ARG_BACKTICKS (`[^`]*`)
ARG_DBLQUOTES (\"([^"\\]|\\.)*\")
ARG_SGLQUOTES ('[^']*')

%%

[ \t]+ ;
{COMMENT}$ ;
({ARG_NOQUOTES}|{ARG_BACKTICKS}|{ARG_DBLQUOTES}|{ARG_SGLQUOTES})+ {
      yylval.singleArg = (char *)malloc((strlen(yytext) + 1) * sizeof(*yylval.singleArg));
      int j = 0;
      bool inSglQuotes = false;
      for (int i = 0; yytext[i] != '\0'; ++i) {
         if (yytext[i] == '\'') {
            inSglQuotes = !inSglQuotes;
            continue;
         } else if (!inSglQuotes) {
            if (yytext[i] == '\\' && yytext[i + 1] != '\0') {
               ++i;
            } else if (yytext[i] == '"') {
               continue;
            }
         }
         yylval.singleArg[j] = yytext[i];
         ++j;
      }
      yylval.singleArg[j] = '\0';
      return SINGLE_ARG;
   }
[0-9]*<&? {
      yylval.redirect = (RedirectInfo){
         REDIR_IN,
         isdigit(yytext[0]) ? atoi(yytext) : STDIN_FILENO,
         yytext[strlen(yytext) - 1] == '&',
      };
      return REDIR;
   }
([0-9]*|&)>&? {
      yylval.redirect = (RedirectInfo){
         REDIR_OUT,
         isdigit(yytext[0]) ? atoi(yytext) : yytext[0] == '&' ? -1 : STDOUT_FILENO,
         yytext[strlen(yytext) - 1] == '&',
      };
      return REDIR;
   }
([0-9]*|&)>> {
      yylval.redirect = (RedirectInfo){
         REDIR_APP,
         isdigit(yytext[0]) ? atoi(yytext) : yytext[0] == '&' ? -1 : STDOUT_FILENO,
      };
      return REDIR;
   }
"||"        return OR;
"&&"        return AND;
<<EOF>>     EndOfFile();
[;|&()\n]   return yytext[0];
.           return ERROR;

%%

int yyparse_string(char * str)
{
   void * buf = yy_create_buffer(NULL, 0);
   yypush_buffer_state(buf);
   yy_scan_string(str);
   int ret = yyparse();
   yypop_buffer_state();
   yy_delete_buffer(buf);
   return ret;
}
