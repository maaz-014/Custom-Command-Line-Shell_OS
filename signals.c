 //SIGINT  (Ctrl+C) — ignored in shell, forwarded to foreground group
//SIGTSTP (Ctrl+Z) — ignored in shell, forwarded to foreground group
//SIGCHLD— reap background children (avoid zombies)
//SIGQUIT— ignored in shell
#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

#include "signals.h"
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

static void sigchld_handler(int sig){
    (void)sig;
    int saved_errno = errno;
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
        if (WIFEXITED(status)) {
            //optional Notification 
            //printf("[done] pid %d exit %d\n", pid, WEXITSTATUS(status));
        }
    }
    errno = saved_errno;
}

static void sigint_handler(int sig){
    (void)sig;
    write(STDOUT_FILENO, "\n", 1);
}

void signals_init(void){
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa, NULL);

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGTSTP, &sa, NULL);

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGQUIT, &sa, NULL);

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGTTOU, &sa, NULL);
    sigaction(SIGTTIN, &sa, NULL);
}

void signals_reset_child(void){
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT,&sa, NULL);
    sigaction(SIGTSTP,&sa, NULL);
    sigaction(SIGQUIT,&sa, NULL);
    sigaction(SIGCHLD,&sa, NULL);
    sigaction(SIGTTOU,&sa, NULL);
    sigaction(SIGTTIN,&sa, NULL);
}