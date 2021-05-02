#pragma once

#include <X11/Xlib.h>

typedef struct Client Client;
struct Client {
	Window frame;
	Window window;
	Client* next;
};

void addClient(Client* client);
Window getClientFrame(Window window);
void removeClient(Window window);