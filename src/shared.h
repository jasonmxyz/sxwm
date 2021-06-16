#pragma once

#include "clients.h"

#include <X11/Xlib.h>
#include <stdbool.h>

// A structure to store the dimensions and client list of a monitor
typedef struct Monitor Monitor;
struct Monitor {
	int width, height;
	Client* clients;
	int clientCount;
};

// The shared memory data structure
typedef struct Shared Shared;
struct Shared {
	int currentTag;
	Window bar;
	bool running;
	char** argv;
	int argc;
	Monitor* monitor;
};

void createSharedMemory();
void attachToSharedMemory();
void detatchFromSharedMemory();
void destroySharedMemory();

extern int sid;
extern Shared* shared;
extern Monitor** monitorList;