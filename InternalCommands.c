#include "InternalCommands.h"
#include "Shell.h"

#include <stdio.h>

int CMD_nothing(int argc, char *const argv[]) { return 0; }

int CMD_exit(int argc, char *const argv[]) {
  if (argc > 2) {
    fprintf(stderr, "%s: too many arguments\n", argv[0]);
    return 1;
  }
  if (argc == 2) {
    shellStatus = atoi(argv[1]);
  }
  EndOfFile();
  return shellStatus;
}

typedef struct InternalCmd {
  char const *name;
  cmd_func_t fct;
} InternalCmd;

InternalCmd const internalCmds[] = {
    // Trié pour bsearch()
    {":", &CMD_nothing},
    {"exit", &CMD_exit},
};

int commandsCmp(void const *p1, void const *p2) {
  InternalCmd const *cmd1 = p1, *cmd2 = p2;
  return strcmp(cmd1->name, cmd2->name);
}

cmd_func_t findCommandFct(char const *name) {
  size_t nbCommands = sizeof(internalCmds) / sizeof(*internalCmds);
  InternalCmd const cmdForCmp = {.name = name},
                    *foundCmd = bsearch(&cmdForCmp, internalCmds, nbCommands,
                                        sizeof(*internalCmds), &commandsCmp);
  return foundCmd == NULL ? NULL : foundCmd->fct;
}
