#pragma once

#include <X11/Xlib.h>
#include <stdbool.h>

typedef struct Client Client;
struct Client {
	Window frame;
	Window window;
	Client* next;
	bool floating;
};

void addClient(Client* client);
Window getClientFrame(Window window);
void removeClient(Window window);
Client* getClient(Window window);