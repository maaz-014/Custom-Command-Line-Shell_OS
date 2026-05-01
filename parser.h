#ifndef PARSER_H
#define PARSER_H

#include "config.h"

typedef struct {
    char* args[MAX_ARGS];   
    int argc;             
    char* input_file;       
    char* output_file;      
    int append;           
}Command;

typedef struct {
    Command cmds[MAX_PIPES];
    int num_cmds;
    int background;      
}Pipeline;


//Returns 0 on success, -1 on parse error. */
int  parse_input(const char *line, Pipeline *pl);

void free_pipeline(Pipeline *pl);

#endif 