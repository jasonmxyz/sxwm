#include "util.h"
#include "shared.h"

#include <stdio.h>
#include <libgen.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <stdarg.h>

#ifdef VERBOSE
void die_(int line, char* file, char* fmt, ...) {
    va_list args;
    printf("%s: ", basename(shared->argv[0]));
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n %s at line %d\n", file, line);
    shared->running = false;
    detatchFromSharedMemory();
    destroySharedMemory();
    exit(1);
}
#else
void die_(char* fmt, ...) {
    va_list args;
    printf("%s: ", basename(shared->argv[0]));
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
    shared->running = false;
    detatchFromSharedMemory();
    destroySharedMemory();
    exit(1);
}
#endif
