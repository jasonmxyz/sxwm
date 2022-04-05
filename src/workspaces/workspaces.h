#pragma once

#include "../clients.h"

#include <X11/Xlib.h>

/* We maintain a linked list of these workspaces on each monitor. */
struct Workspace {
	int x;
	int y;
	int width;
	int height;
	Client *clients;
	Client *focused;
	int clientCount;
	struct WorkspaceDescription *wd;
	struct Workspace *prev;
	struct Workspace *next;
};

/* We maintain a linked list of these structures representing the avialable
 * types of workspace. */
struct WorkspaceDescription {
	int id;
	char *name;
	struct WorkspaceDescription *prev;
	struct WorkspaceDescription *next;
	void (*newClient)(struct Workspace*, Window);
};