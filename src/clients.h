#pragma once

#include "workspaces.h"

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
int getClientWorkspace(Window window, struct Client **retClient, struct Workspace **retWorkspace);
int getClientWorkspaceAny(Window window, struct Client **retClient, struct Workspace **retWorkspace);