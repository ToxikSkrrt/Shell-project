#ifndef _INTERNAL_COMMANDS_H
#define _INTERNAL_COMMANDS_H

typedef int (*cmd_func_t)(int argc, char *const argv[]);

cmd_func_t findCommandFct(char const *name);

#endif
