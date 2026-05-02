#ifndef SIGNALS_H
#define SIGNALS_H

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
void signals_init(void);

void signals_reset_child(void);

#endif