#pragma once

#include "util.h"

#include <X11/Xlib.h>
#include <stdbool.h>

typedef struct Client Client;
struct Client {
	Window frame;
	Window window;
	Client* next;
	Client* previous;
	bool floating;
	int tag;
	Point floatingLocation;
};

// A structure to store the dimensions and client list of a monitor
typedef struct Monitor Monitor;
struct Monitor {
	int width, height;
	Client* clients;
	int clientCount;
};

void addClient(Client* client);
Window getClientFrame(Window window);
void removeClient(Window window);
Client* getClientByWindow(Window window);