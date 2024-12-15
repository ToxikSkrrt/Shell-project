#define _GNU_SOURCE

#include "Evaluation.h"
#include "Shell.h"

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>

enum
{
  R,
  W
};

int cmd_simple(Expression *expr)
{
  errno = 0;
  int status;
  pid_t pid = fork();
  if (!pid)
  {
    execvp(expr->argv[0], expr->argv);
    exit(errno);
  }
  else
  {
    waitpid(pid, &status, 0);
    if (WIFEXITED(status))
      return WEXITSTATUS(status);
    else if (WIFSIGNALED(status))
      return WTERMSIG(status) == SIGSEGV ? WTERMSIG(status) + 128 : WTERMSIG(status);
    else
      return EXIT_FAILURE;
  }
}

void redirect(Expression *expr)
{
  int fd;
  switch (expr->redirect.type)
  {
  case REDIR_OUT:
    fd = open(expr->redirect.fileName, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    break;
  case REDIR_IN:
    fd = open(expr->redirect.fileName, O_RDONLY);
    break;
  case REDIR_APP:
    fd = open(expr->redirect.fileName, O_WRONLY | O_CREAT, 0666);
    lseek(fd, 0, SEEK_END);
    break;
  }

  if (expr->redirect.fd == 0 || expr->redirect.fd == 1 || expr->redirect.fd == 2)
  {
    int fd_backup = dup(expr->redirect.fd);
    dup2(fd, expr->redirect.fd);
    close(fd);
    evaluateExpr(expr->left);
    dup2(fd_backup, expr->redirect.fd);
    close(fd_backup);
  }
  else if (expr->redirect.fd == -1)
  {
    int fd_backup_1 = dup(STDOUT_FILENO);
    int fd_backup_2 = dup(STDERR_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);
    close(fd);
    evaluateExpr(expr->left);
    dup2(fd_backup_1, STDOUT_FILENO);
    dup2(fd_backup_2, STDERR_FILENO);
    close(fd_backup_1);
    close(fd_backup_2);
  }
}

void sequence(Expression *expr, int mode)
{
  int status = evaluateExpr(expr->left);

  if (mode == 0 || (mode == 1 && !status) || (mode == 2 && status))
    evaluateExpr(expr->right);
}

int pipe_shell(Expression *expr)
{
  int result = 0;
  int tube[2];
  pipe(tube);

  pid_t pid = fork();
  if (!pid)
  {
    close(tube[R]);
    dup2(tube[W], STDOUT_FILENO);
    result = evaluateExpr(expr->left);
    exit(result);
  }
  else
  {
    close(tube[W]);
    int backup = dup(STDIN_FILENO);
    dup2(tube[R], STDIN_FILENO);

    result = evaluateExpr(expr->right);

    dup2(backup, STDIN_FILENO);
    close(backup);
    close(tube[R]);
    return result;
  }
}

void bg(Expression *expr)
{
  errno = 0;
  pid_t pid = fork();
  if (!pid)
  {
    evaluateExpr(expr->left);
    exit(errno);
  }
}

void traitant(int s)
{
  pid_t pid;
  while ((pid = waitpid(-1, 0, WNOHANG)) > 0)
    ;
}

int evaluateExpr(Expression *expr)
{
  static int first = 1;
  sigset_t m;
  sigfillset(&m);
  sigprocmask(SIG_BLOCK, &m, NULL);

  if (first)
  {
    // code d'initialisation
    struct sigaction sa;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = traitant;
    sigaction(SIGCHLD, &sa, NULL);

    first = 0;
  }

  switch (expr->type)
  {
  case ET_EMPTY:
    break;
  case ET_SIMPLE:
    shellStatus = cmd_simple(expr);
    break;
  case ET_REDIRECT:
    redirect(expr);
    break;
  case ET_SEQUENCE:
    sequence(expr, 0);
    break;
  case ET_SEQUENCE_AND:
    sequence(expr, 1);
    break;
  case ET_SEQUENCE_OR:
    sequence(expr, 2);
    break;
  case ET_PIPE:
    shellStatus = pipe_shell(expr);
    break;
  case ET_BG:
    bg(expr);
    break;

  default:
    fprintf(stderr, "sorry, this shell is not yet implemented\n");
    shellStatus = 1;
    break;
  }

  sigprocmask(SIG_UNBLOCK, &m, NULL);

  return shellStatus;
}