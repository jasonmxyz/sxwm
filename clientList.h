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

void addClient(Client* client);
Window getClientFrame(Window window);
void removeClient(Window window);
Client* getClientByWindow(Window window);