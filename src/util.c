#include "util.h"
#include "sxwm.h"

#include <stdio.h>
#include <libgen.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/mman.h>

extern char** g_argv;

void diel(char* fmt, ...) {
    va_list args;

    printf("%s: ", basename(g_argv[0]));

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    printf("\n");

    if (shmName != NULL) shm_unlink(shmName);

    exit(1);
}

#ifdef VERBOSE
void dief(int line, char* file, char* fmt, ...) {
    va_list args;

    printf("%s: ", basename(g_argv[0]));

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    printf("\n %s at line %d\n", file, line);

    if (shmName != NULL) shm_unlink(shmName);

    exit(1);
}

void debugf(int line, char* file, char* fmt, ...) {
    va_list args;

    printf("%s at line %d: ", file, line);

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    printf("\n");
}

#endif