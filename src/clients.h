#pragma once

#include "util.h"

#include <X11/Xlib.h>
#include <stdbool.h>

typedef struct Point Point;
typedef Point Dimension;
struct Point {
	int x, y;
};

typedef struct Client Client;
struct Client {
	Window frame;
	Window window;
	Client* next;
	Client* previous;
	bool floating;
	int tags;
	Point floatingLocation;
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
	int currentTags;
	bool running;
	Monitor* monitor;
};

void addClient(Client* client);
Window getClientFrame(Window window);
void removeClient(Window window);
Client* getClientByWindow(Window window);

extern Shared* shared;
extern Monitor** monitorList;