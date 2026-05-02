#ifndef REDIRECT_H
#define REDIRECT_H

#include "parser.h"

int apply_redirections(const Command *cmd);

int setup_pipe_write(int pipefd[2]);
int setup_pipe_read (int pipefd[2]);

#endif

