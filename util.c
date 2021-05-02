#include "global.h"

#include <stdio.h>
#include <libgen.h>
#include <stdlib.h>

extern char** g_argv;

#ifdef VERBOSE
void die_(int line, char* file, char* message) {
    printf("%s: %s\n %s at line %d\n", basename(g_argv[0]), message, file, line);
    exit(1);
}
#else
void die_(char* message) {
    printf("%s: %s\n", basename(g_argv[0]), message);
    exit(1);
}
#endif
