#include "header.h"

extern char** g_argv;

void die_(int line, char* file, char* message) {
    printf("%s: %s\n %s at line %d\n", basename(g_argv[0]), message, file, line);
    exit(1);
}