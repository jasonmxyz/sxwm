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