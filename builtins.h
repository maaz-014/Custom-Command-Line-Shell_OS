#ifndef BUILTINS_H
#define BUILTINS_H

#include "parser.h"

void history_add(const char *line);
void history_show(void);
void history_free(void);

int  run_builtin(const Command *cmd, int *exit_code);

int  is_builtin(const char *name);

#endif 