#ifndef CONFIG_H
#define CONFIG_H

#define SHELL_NAME        "myshell"
#define SHELL_VERSION     "1.0.0"
#define SHELL_PROMPT      "myshell> "

#define STUDENT_1_NAME    "Muhammad Maaz"
#define STUDENT_1_ID      "24K-0968"
#define STUDENT_2_NAME    "Fahad Zuberi"
#define STUDENT_2_ID      "24K-0717"
#define STUDENT_3_NAME    "Muhammad Saad Khan"
#define STUDENT_3_ID      "24K-0680"

// Limits
#define MAX_INPUT_LEN     4096
#define MAX_ARGS          256
#define MAX_PIPES         64
#define MAX_HISTORY       100
#define MAX_PATH_LEN      1024
#define MAX_JOBS          64

//Token types 
#define TOK_WORD          0
#define TOK_PIPE          1
#define TOK_REDIR_IN      2   
#define TOK_REDIR_OUT     3   
#define TOK_REDIR_APPEND  4   
#define TOK_BACKGROUND    5   
#define TOK_EOF           6

#define JOB_RUNNING       0
#define JOB_STOPPED       1
#define JOB_DONE          2

//ANSI colour codes
#define ANSI_RESET        "\033[0m"
#define ANSI_BOLD         "\033[1m"
#define ANSI_RED          "\033[31m"
#define ANSI_GREEN        "\033[32m"
#define ANSI_YELLOW       "\033[33m"
#define ANSI_BLUE         "\033[34m"
#define ANSI_MAGENTA      "\033[35m"
#define ANSI_CYAN         "\033[36m"
#define ANSI_WHITE        "\033[37m"
#define ANSI_BRIGHT_GREEN "\033[92m"
#define ANSI_BRIGHT_CYAN  "\033[96m"

#endif 