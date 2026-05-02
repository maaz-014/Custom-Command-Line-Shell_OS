#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    int   type;          
    char *value;         
} Token;

#define MAX_TOKENS (MAX_ARGS * MAX_PIPES + 64)

static Token tokens[MAX_TOKENS];
static int   num_tokens = 0;

static char *expand_env(const char *word){
    char buf[MAX_INPUT_LEN];
    int  bi = 0;
    int  i  = 0;
    int  len = (int)strlen(word);

    while (i < len && bi < (int)sizeof(buf) - 1) {
        if (word[i] == '$') {
            i++;
            char varname[256];
            int  vi = 0;
            while (i < len && (isalnum((unsigned char)word[i]) || word[i] == '_')) varname[vi++] = word[i++];
            
            varname[vi] = '\0';

            const char *val = (vi > 0) ? getenv(varname) : NULL;
            if (val) {
                int vlen = (int)strlen(val);
                if (bi + vlen < (int)sizeof(buf) - 1) {
                    memcpy(buf + bi, val, vlen);
                    bi += vlen;
                }
            } else if (vi == 0) {
                buf[bi++] = '$';
            }
            
        } else {
            buf[bi++] = word[i++];
        }
    }
    buf[bi] = '\0';
    return strdup(buf);
}

static int tokenise(const char *line){
    num_tokens = 0;
    int i = 0;
    int len = (int)strlen(line);

    while (i < len) {
        if (isspace((unsigned char)line[i])) { i++; continue; }

        if (line[i] == '#') break;

        if (line[i] == '|') {
            tokens[num_tokens++] = (Token){ TOK_PIPE, NULL };
            i++; continue;
        }
        if (line[i] == '&') {
            tokens[num_tokens++] = (Token){ TOK_BACKGROUND, NULL };
            i++; continue;
        }
        if (line[i] == '<') {
            tokens[num_tokens++] = (Token){ TOK_REDIR_IN, NULL };
            i++; continue;
        }
        if (line[i] == '>') {
            if (i + 1 < len && line[i+1] == '>') {
                tokens[num_tokens++] = (Token){ TOK_REDIR_APPEND, NULL };
                i += 2;
            } else {
                tokens[num_tokens++] = (Token){ TOK_REDIR_OUT, NULL };
                i++;
            }
            continue;
        }
        
        char buf[MAX_INPUT_LEN];
        int  bi = 0;
        while (i < len && !isspace((unsigned char)line[i]) &&
               line[i] != '|' && line[i] != '&' &&
               line[i] != '<' && line[i] != '>' &&
               line[i] != '#')
        {
            if (line[i] == '\'') {
                i++;
                while (i < len && line[i] != '\'')
                    buf[bi++] = line[i++];
                if (i < len) i++; 
            } else if (line[i] == '"') {
                
                i++;
                while (i < len && line[i] != '"') {
                    if (line[i] == '$') {
                        i++;
                        char varname[256];
                        int  vi = 0;
                        while (i < len && (isalnum((unsigned char)line[i]) || line[i] == '_')) varname[vi++] = line[i++];
                        
                        varname[vi] = '\0';
                        const char *val = (vi > 0) ? getenv(varname) : NULL;
                        if (val) {
                            int fl = (int)strlen(val);
                            if (bi + fl < (int)sizeof(buf) - 1) {
                                memcpy(buf + bi, val, fl);
                                bi += fl;
                            }
                        } else if (vi == 0) {
                            buf[bi++] = '$'; 
                        }
                    } else {
                        buf[bi++] = line[i++];
                    }
                }
                if (i < len) i++;
            } else {
                buf[bi++] = line[i++];
            }
        }
        buf[bi] = '\0';

        char *expanded = expand_env(buf);
        tokens[num_tokens++] = (Token){ TOK_WORD, expanded };
    }

    tokens[num_tokens] = (Token){ TOK_EOF, NULL };
    return 0;
}

int parse_input(const char *line, Pipeline *pl){
    memset(pl, 0, sizeof(*pl));

    if (!line || !*line) return -1;

    if (tokenise(line) != 0) return -1;

    int ti = 0;
    int ci = 0;  

    while (tokens[ti].type != TOK_EOF) {
        if (ci >= MAX_PIPES) {
            fprintf(stderr, ANSI_RED "myshell: too many pipe stages\n" ANSI_RESET);
            return -1;
        }
        Command *cmd = &pl->cmds[ci];

        while (tokens[ti].type != TOK_EOF &&
               tokens[ti].type != TOK_PIPE &&
               tokens[ti].type != TOK_BACKGROUND)
        {
            int t = tokens[ti].type;

            if (t == TOK_WORD) {
                if (cmd->argc >= MAX_ARGS - 1) {
                    fprintf(stderr, ANSI_RED "myshell: too many arguments\n" ANSI_RESET);
                    return -1;
                }
                cmd->args[cmd->argc++] = tokens[ti].value;
                tokens[ti].value = NULL;
                ti++;
            } else if (t == TOK_REDIR_IN) {
                ti++;
                if (tokens[ti].type != TOK_WORD) {
                    fprintf(stderr, ANSI_RED "myshell: expected filename after '<'\n" ANSI_RESET);
                    return -1;
                }
                cmd->input_file = tokens[ti].value;
                tokens[ti].value = NULL;
                ti++;
            } else if (t == TOK_REDIR_OUT || t == TOK_REDIR_APPEND) {
                cmd->append = (t == TOK_REDIR_APPEND) ? 1 : 0;
                ti++;
                if (tokens[ti].type != TOK_WORD) {
                    fprintf(stderr, ANSI_RED "myshell: expected filename after '>'\n" ANSI_RESET);
                    return -1;
                }
                cmd->output_file = tokens[ti].value;
                tokens[ti].value = NULL;
                ti++;
            } else {
                ti++;
            }
        }
        cmd->args[cmd->argc] = NULL;

        if (cmd->argc > 0) ci++;

        if (tokens[ti].type == TOK_PIPE) {
            ti++;
        } else if (tokens[ti].type == TOK_BACKGROUND) {
            pl->background = 1;
            ti++;
        }
    }

    pl->num_cmds = ci;

    for (int i = 0; i <= num_tokens; i++) {
        if (tokens[i].value) { free(tokens[i].value); tokens[i].value = NULL; }
    }

    return (pl->num_cmds > 0) ? 0 : -1;
}

void free_pipeline(Pipeline *pl){
    for (int i = 0; i < pl->num_cmds; i++) {
        Command *cmd = &pl->cmds[i];
        for (int j = 0; j < cmd->argc; j++) {
            free(cmd->args[j]);
            cmd->args[j] = NULL;
        }
        free(cmd->input_file);
        free(cmd->output_file);
        cmd->input_file  = NULL;
        cmd->output_file = NULL;
    }
    pl->num_cmds = 0;
}