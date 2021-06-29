#pragma once

#include "util.h"

#include <X11/Xlib.h>

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
	Client* focusNext;
	Client* focusPrevious;
	int floating;
	int tags;
	Point floatingLocation;
};

// A structure to store the dimensions and client list of a monitor
typedef struct Monitor Monitor;
struct Monitor {
	int width, height;
	Client* clients;
	Client* focused;
	int clientCount;
};

void addClient(Client* client);
void removeClient(Client* client);
void frameClient(Client* client);
void destroyFrame(Client* client);
Client* getClient(Window window, int isFrame);

#ifdef VERBOSE
#define DUMPCLIENTS() dumpClients()
void dumpClients();
#else
#define DUMPCLIENTS()
#endif