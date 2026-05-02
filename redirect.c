#include "redirect.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

int apply_redirections(const Command *cmd){

    if (cmd->input_file) {
        int fd = open(cmd->input_file, O_RDONLY);
        if (fd < 0) {
            fprintf(stderr,ANSI_RED "%s: %s\n" ANSI_RESET,cmd->input_file, strerror(errno));
            return -1;
        }
        if (dup2(fd, STDIN_FILENO) < 0) {
            perror("dup2 (stdin)");
            close(fd);
            return -1;
        }
        close(fd);
    }

    if (cmd->output_file) {
        int flags = O_WRONLY | O_CREAT;
        flags |= cmd->append ? O_APPEND : O_TRUNC;
        int fd = open(cmd->output_file, flags, 0644);
        if (fd < 0) {
            fprintf(stderr,ANSI_RED "%s: %s\n" ANSI_RESET,cmd->output_file, strerror(errno));
            return -1;
        }
        if (dup2(fd, STDOUT_FILENO) < 0) {
            perror("dup2 (stdout)");
            close(fd);
            return -1;
        }
        close(fd);
    }

    return 0;
}

int setup_pipe_write(int pipefd[2]){
    if (dup2(pipefd[1], STDOUT_FILENO) < 0) {
        perror("dup2 pipe write");
        return -1;
    }
    close(pipefd[0]);
    close(pipefd[1]);
    return 0;
}


int setup_pipe_read(int pipefd[2]){
    if (dup2(pipefd[0], STDIN_FILENO) < 0) {
        perror("dup2 pipe read");
        return -1;
    }
    close(pipefd[0]);
    close(pipefd[1]);
    return 0;
}