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

void createSharedMemory();
void* smalloc(int length);
void* scalloc(int length);
void sfree(void* addr);