#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include<pthread.h>

#include "config.h"
#include "parser.h"
#include "executor.h"
#include "builtins.h"
#include "signals.h"

static void print_banner(void){
    printf(ANSI_BOLD ANSI_BRIGHT_GREEN
"\n"
"  =============================================================\n"
"  ||                                                         ||\n"
"  ||                       M Y S H E L L                     ||\n"
"  ||                                                         ||\n"
"  =============================================================\n"
    ANSI_RESET);

    printf(ANSI_CYAN
"  ┌─────────────────────────────────────────────────────────────┐\n"
"  │   Custom UNIX-like Shell  v%-6s  CS-2006 Spring 2026      │\n"
"  │                                                             │\n"
"  │   Kernel Config Identifier (Group Members):                 │\n"
"  │     %-20s  %-12s  Shell Core          │\n"
"  │     %-20s  %-12s  Pipes/Redirs        │\n"
"  │     %-20s  %-12s  Signals/Jobs        │\n"
"  │                                                             │\n"
"  │   Type 'help' for built-in commands.  Ctrl+D to exit.       │\n"
"  └─────────────────────────────────────────────────────────────┘\n"
    ANSI_RESET "\n",
    SHELL_VERSION,
    STUDENT_1_NAME, STUDENT_1_ID,
    STUDENT_2_NAME, STUDENT_2_ID,
    STUDENT_3_NAME, STUDENT_3_ID);
}

//myshell:~/src/project> 
static void print_prompt(void){
    char cwd[MAX_PATH_LEN];
    const char *home = getenv("HOME");

    if (getcwd(cwd, sizeof(cwd)) == NULL) strncpy(cwd, "?", sizeof(cwd));

    // Abbreviate $HOME as ~
    char display[MAX_PATH_LEN];
    if (home && strncmp(cwd, home, strlen(home)) == 0) {
        snprintf(display, sizeof(display), "~%s", cwd + strlen(home));
    } else {
        strncpy(display, cwd, sizeof(display));
    }

    printf(ANSI_BOLD ANSI_BRIGHT_GREEN SHELL_NAME ANSI_RESET
           ANSI_BOLD ":" ANSI_RESET
           ANSI_BRIGHT_CYAN "%s" ANSI_RESET
           ANSI_BOLD ANSI_YELLOW "$ " ANSI_RESET,display);
    
    fflush(stdout);
}

//Strip trailing newline / carriage-return from a string.
static void chomp(char *s){
    int len = (int)strlen(s);
    while (len > 0 && (s[len-1] == '\n' || s[len-1] == '\r'))
        s[--len] = '\0';
}


int main(void){
    signals_init();

    pid_t shell_pgid = getpid();
    setpgid(shell_pgid, shell_pgid);
    tcsetpgrp(STDIN_FILENO, shell_pgid);

    print_banner();

    char input[MAX_INPUT_LEN];
    int last_exit = 0;
    int interactive = isatty(STDIN_FILENO);

    for (;;) {
        if (interactive)
            print_prompt();

        if (fgets(input, sizeof(input), stdin) == NULL) {

            if (interactive) printf("\n");
            break;
        }

        chomp(input);

        if (input[0] == '\0') continue;

        history_add(input);

        Pipeline pl;
        if (parse_input(input, &pl) != 0)
            continue;

        last_exit = execute_pipeline(&pl);

        free_pipeline(&pl);
    }

    history_free();
    return last_exit;
}