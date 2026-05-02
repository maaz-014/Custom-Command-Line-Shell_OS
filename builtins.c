#include "builtins.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

static char *history[MAX_HISTORY];
static int   history_count = 0;

void history_add(const char *line){
    if (!line || !*line) return;
    if (history_count < MAX_HISTORY) {
        history[history_count++] = strdup(line);
    } else {
        free(history[0]);
        memmove(history, history + 1, (MAX_HISTORY - 1) * sizeof(char *));
        history[MAX_HISTORY - 1] = strdup(line);
    }
}

void history_show(void){
    for (int i = 0; i < history_count; i++)
        printf(ANSI_CYAN "%4d" ANSI_RESET "  %s\n", i + 1, history[i]);
}

void history_free(void){
    for (int i = 0; i < history_count; i++) {
        free(history[i]);
        history[i] = NULL;
    }
    history_count = 0;
}

static const char *builtin_names[]={
    "cd", "pwd", "exit", "history", "help", "env",
    "unset", "export", "exec", NULL
};

int is_builtin(const char *name){
    if (!name) return 0;
    for (int i = 0; builtin_names[i]; i++)
        if (strcmp(name, builtin_names[i]) == 0) return 1;
    return 0;
}

static int builtin_cd(const Command *cmd){
    const char *dir;
    if (cmd->argc < 2) {
        dir = getenv("HOME");
        if (!dir) {
            fprintf(stderr, ANSI_RED "cd: HOME not set\n" ANSI_RESET);
            return 1;
        }
    } 
    else if (strcmp(cmd->args[1], "-") == 0) {
        dir = getenv("OLDPWD");
        if (!dir) {
            fprintf(stderr, ANSI_RED "cd: OLDPWD not set\n" ANSI_RESET);
            return 1;
        }
        printf("%s\n", dir);
    }
    else {
        dir = cmd->args[1];
    }
    char cwd[MAX_PATH_LEN];
    if (getcwd(cwd, sizeof(cwd)))
        setenv("OLDPWD", cwd, 1);

    if (chdir(dir) != 0) {
        fprintf(stderr, ANSI_RED "cd: %s: %s\n" ANSI_RESET, dir, strerror(errno));
        return 1;
    }
    if (getcwd(cwd, sizeof(cwd)))
        setenv("PWD", cwd, 1);

    return 0;
}
static int builtin_pwd(void){
    char cwd[MAX_PATH_LEN];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        fprintf(stderr, ANSI_RED "pwd: %s\n" ANSI_RESET, strerror(errno));
        return 1;
    }
    printf(ANSI_BRIGHT_CYAN "%s\n" ANSI_RESET, cwd);
    return 0;
}

static int builtin_history(const Command *cmd){
    if (cmd->argc >= 2) {
        int n = atoi(cmd->args[1]);
        int start = history_count - n;
        if (start < 0) start = 0;
        for (int i = start; i < history_count; i++)
            printf(ANSI_CYAN "%4d" ANSI_RESET "  %s\n", i + 1, history[i]);
    } else {
        history_show();
    }
    return 0;
}

static int builtin_help(void){
    printf(ANSI_BOLD ANSI_BRIGHT_GREEN"━━━  %s v%s  Built-in Commands  ━━━\n" ANSI_RESET,SHELL_NAME, SHELL_VERSION);
    printf("  " ANSI_YELLOW "cd" ANSI_RESET "      [dir]      Change directory (default: HOME)\n");
    printf("  " ANSI_YELLOW "cd -" ANSI_RESET "              Return to previous directory\n");
    printf("  " ANSI_YELLOW "pwd" ANSI_RESET "               Print working directory\n");
    printf("  " ANSI_YELLOW "history" ANSI_RESET " [n]       Show command history (last n)\n");
    printf("  " ANSI_YELLOW "help" ANSI_RESET "              Show this help message\n");
    printf("  " ANSI_YELLOW "env" ANSI_RESET "               List environment variables\n");
    printf("  " ANSI_YELLOW "export" ANSI_RESET " VAR=VAL   Set environment variable\n");
    printf("  " ANSI_YELLOW "unset" ANSI_RESET "  VAR       Unset environment variable\n");
    printf("  " ANSI_YELLOW "exit" ANSI_RESET " [code]      Exit the shell\n");
    printf("\n  " ANSI_BOLD "Operators:" ANSI_RESET "  |   >   >>   <   &   $VAR\n");
    return 0;
} // executes on help (command)!

extern char **environ;

static int builtin_env(void){
    for (char **e = environ; *e; e++)
        printf("%s\n", *e);
    return 0;
}

static int builtin_export(const Command *cmd){
    if (cmd->argc < 2){
        builtin_env();
        return 0; 
    }
    for (int i = 1; i < cmd->argc; i++) {
        char *eq = strchr(cmd->args[i], '=');
        if (eq) {
            char name[256];
            int nl = (int)(eq - cmd->args[i]);
            strncpy(name, cmd->args[i], nl);
            name[nl] = '\0';
            if (setenv(name, eq + 1, 1) != 0) {
                fprintf(stderr, ANSI_RED "export: %s\n" ANSI_RESET, strerror(errno));
                return 1;
            }
        } else {
            //mark existing variable — no-op here 
        }
    }
    return 0;
}

static int builtin_unset(const Command *cmd){
    if (cmd->argc < 2) {
        fprintf(stderr, ANSI_RED "unset: expected variable name\n" ANSI_RESET);
        return 1;
    }
    for (int i = 1; i < cmd->argc; i++) unsetenv(cmd->args[i]);
    return 0;
}

static int builtin_exec(const Command *cmd)
{
    if (cmd->argc < 2) {
        return 0;
    }
    
    execvp(cmd->args[1], &cmd->args[1]);
    
    fprintf(stderr, ANSI_RED "exec: %s: %s\n" ANSI_RESET, cmd->args[1], strerror(errno));
    return 1;
}

int run_builtin(const Command *cmd, int *exit_code){
    if (!cmd || cmd->argc == 0) return 0;
    const char *name = cmd->args[0];

    if (strcmp(name, "cd")      == 0) { *exit_code = builtin_cd(cmd);   return 1; }
    if (strcmp(name, "pwd")     == 0) { *exit_code = builtin_pwd();      return 1; }
    if (strcmp(name, "history") == 0) { *exit_code = builtin_history(cmd); return 1; }
    if (strcmp(name, "help")    == 0) { *exit_code = builtin_help();     return 1; }
    if (strcmp(name, "env")     == 0) { *exit_code = builtin_env();      return 1; }
    if (strcmp(name, "export")  == 0) { *exit_code = builtin_export(cmd); return 1; }
    if (strcmp(name, "unset")   == 0) { *exit_code = builtin_unset(cmd); return 1; }
    if (strcmp(name, "exec")    == 0) { *exit_code = builtin_exec(cmd);  return 1; }
    if (strcmp(name, "exit")    == 0) {
        int code = 0;
        if (cmd->argc >= 2) code = atoi(cmd->args[1]);
        history_free();
        printf(ANSI_YELLOW "Goodbye!\n" ANSI_RESET);
        exit(code);
    }

    return 0; 
}