#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <libgen.h>

#define die(X) die_(__LINE__, __FILE__, X)

void die_(int line, char* file, char* message);