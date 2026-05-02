#include "executor.h"
#include "builtins.h"
#include "redirect.h"
#include "signals.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>

int execute_pipeline(Pipeline *pl){
    if (!pl || pl->num_cmds == 0) return 0;

    if (pl->num_cmds == 1) {
        int exit_code = 0;
        if (run_builtin(&pl->cmds[0], &exit_code)) return exit_code;
    }

    int n= pl->num_cmds;
    int background = pl->background;

    int (*pipes)[2] = NULL;
    if (n > 1) {
        pipes = malloc((n - 1) * sizeof(int[2]));
        if (!pipes) { perror("malloc"); return 1; }

        for (int i = 0; i < n - 1; i++) {
            if (pipe(pipes[i]) < 0) {
                perror("pipe");

                for (int j = 0; j < i; j++) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
                free(pipes);
                return 1;
            }
        }
    }

    pid_t pids[MAX_PIPES];
    pid_t pgid = 0;

    for (int i = 0; i < n; i++) {
        int in_fd  = (i == 0) ? -1 : pipes[i-1][0];
        int out_fd = (i == n - 1) ? -1 : pipes[i][1];

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");

            for (int j = 0; j < n - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            free(pipes);
            return 1;
        }

        if (pid == 0) {
            if (pgid == 0) pgid = getpid();
            setpgid(0, pgid);

            signals_reset_child();

            if (in_fd != -1) {
                if (dup2(in_fd, STDIN_FILENO) < 0) {
                    perror("dup2 in");
                    exit(1);
                }
            }
            
            if (out_fd != -1) {
                if (dup2(out_fd, STDOUT_FILENO) < 0) {
                    perror("dup2 out");
                    exit(1);
                }
            }
            if (pipes) {
                for (int j = 0; j < n - 1; j++) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
            }

            if (apply_redirections(&pl->cmds[i]) < 0) exit(1);

            int ec = 0;
            if (run_builtin(&pl->cmds[i], &ec)) exit(ec);
            
            if (pl->cmds[i].argc == 0) exit(0);

            execvp(pl->cmds[i].args[0], pl->cmds[i].args);

            fprintf(stderr,ANSI_RED "%s: command not found\n" ANSI_RESET,pl->cmds[i].args[0]);
            exit(127);
        }
        if (pgid == 0) pgid = pid;
        setpgid(pid, pgid);

        pids[i] = pid;
    }

    if (pipes) {
        for (int j = 0; j < n - 1; j++) {
            close(pipes[j][0]);
            close(pipes[j][1]);
        }
        free(pipes);
    }

    if (background) {
        printf(ANSI_YELLOW "[bg] pid %d\n" ANSI_RESET, pgid);
        return 0;
    }

    tcsetpgrp(STDIN_FILENO, pgid);

    int last_status = 0;
    for (int i = 0; i < n; i++) {
        int status;
        if (waitpid(pids[i], &status, WUNTRACED) < 0) {
            if (errno != ECHILD) perror("waitpid");
            continue;
        }

        if (WIFSTOPPED(status)) {
            printf(ANSI_YELLOW "\n[stopped] pid %d\n" ANSI_RESET, pids[i]);
        }

        if (i == n - 1) {
            if (WIFEXITED(status))
                last_status = WEXITSTATUS(status);
            else if (WIFSIGNALED(status))
                last_status = 128 + WTERMSIG(status);
        }
    }

    tcsetpgrp(STDIN_FILENO, getpgrp());

    return last_status;
}