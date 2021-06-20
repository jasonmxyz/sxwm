#pragma once

#include <X11/Xlib.h>

#ifdef VERBOSE

#define die(X, ...) dief(__LINE__, __FILE__, X, ##__VA_ARGS__)
void dief(int line, char* file, char* fmt, ...);
#define DEBUG(X, ...) debugf(__LINE__, __FILE__, X, ##__VA_ARGS__)
void debugf(int line, char* file, char* fmt, ...);

#else

#define die(X, ...) diel(X, ##__VA_ARGS__)
void diel(char* fmt, ...);
#define DEBUG(X, ...)

#endif