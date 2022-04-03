#pragma once

#include <X11/Xlib.h>

#define die(X, ...) diel(X, ##__VA_ARGS__)
void diel(char* fmt, ...);