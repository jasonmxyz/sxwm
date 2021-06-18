#pragma once

#include "clients.h"

#include <X11/Xlib.h>
#include <stdbool.h>

// Data about a allocated page
typedef struct PageInfo PageInfo;
struct PageInfo {
	int size;
	PageInfo* prev;
	PageInfo* next;
};

// Data about a specific shared memory allocation
typedef struct SMallocInfo SMallocInfo;
struct SMallocInfo {
	int size;
	int available;
	SMallocInfo* prev;
};


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
void* smalloc(int length);
void* scalloc(int length);
void sfree(void* addr);

extern Shared* shared;
extern Monitor** monitorList;