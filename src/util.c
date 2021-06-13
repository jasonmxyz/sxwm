#include "util.h"

#include <stdio.h>
#include <libgen.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <stdarg.h>

extern char** g_argv;
extern int sid;
extern Shared* shared;

#ifdef VERBOSE
void die_(int line, char* file, char* message) {
    printf("%s: %s\n %s at line %d\n", basename(g_argv[0]), message, file, line);
    shmdt(shared);
    shmctl(sid, IPC_RMID, 0);
    exit(1);
}
void dief_(int line, char* file, char* fmt, ...) {
    va_list args;
    printf("%s: ", basename(g_argv[0]));
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n %s at line %d\n", file, line);
    shmdt(shared);
    shmctl(sid, IPC_RMID, 0);
    exit(1);
}
#else
void die_(char* message) {
    printf("%s: %s\n", basename(g_argv[0]), message);
    shmdt(shared);
    shmctl(sid, IPC_RMID, 0);
    exit(1);
}
void dief_(char* fmt, ...) {
    va_list args;
    printf("%s: ", basename(g_argv[0]));
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
    shmdt(shared);
    shmctl(sid, IPC_RMID, 0);
    exit(1);
}
#endif
