#pragma once

#include <X11/Xlib.h>
#include <stdbool.h>

#ifdef VERBOSE
#define die(X) die_(__LINE__, __FILE__, X)
#define dief(X, ...) dief_(__LINE__, __FILE__, X, ##__VA_ARGS__)
void die_(int line, char* file, char* message);
void dief_(int line, char* file, char* fmt, ...);
#else
#define die(X) die_(X)
void die_(char* message);
void dief_(char* fmt, ...);
#endif

typedef struct Point Point;
typedef Point Dimension;
struct Point {
	int x, y;
};

typedef struct Shared Shared;
struct Shared {
	int currentTag;
	Window bar;
	bool running;
};