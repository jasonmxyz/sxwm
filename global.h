#pragma once

#ifdef VERBOSE
#define die(X) die_(__LINE__, __FILE__, X)
void die_(int line, char* file, char* message);
#else
#define die(X) die_(X)
void die_(char* message);
#endif