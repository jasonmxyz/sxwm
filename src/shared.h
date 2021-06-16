#pragma once

#include <X11/Xlib.h>
#include <stdbool.h>

typedef struct Shared Shared;
struct Shared {
	int currentTag;
	Window bar;
	bool running;
	char** argv;
	int argc;
};

void createSharedMemory();
void attachToSharedMemory();
void detatchFromSharedMemory();
void destroySharedMemory();

extern int sid;
extern Shared* shared;