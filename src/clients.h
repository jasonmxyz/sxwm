#pragma once

#include <X11/Xlib.h>

struct Client {
	Window frame;
	Window window;
	struct Client *next;
	struct Client *previous;
	struct Client *focusNext;
	struct Client *focusPrevious;
	int floating;
	int tags;
	int floatingx;
	int floatingy;
};

void removeClient(struct Client *client);
void frameClient(struct Client *client);
void destroyFrame(struct Client *client);
struct Client *getClient(Window window, int isFrame);