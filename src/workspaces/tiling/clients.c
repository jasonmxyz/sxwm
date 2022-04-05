#include "tiling.h"
#include "../workspaces.h"
#include "../../clients.h"
#include "../../sxwm.h"

#include <stdlib.h>
#include <X11/Xlib.h>

extern Display* display;
extern void tile();

/*
 * Add some window to this workspace.
 *
 * Create a client structure for this window, create a frame and reparent the
 * window within it, then add the client to the list within this workspace.
 */
void newClient(struct Workspace *workspace, Window window)
{
	Client *client = malloc(sizeof(Client));
	client->frame    = 0;
	client->window   = window;
	client->floating = 0;
	client->tags     = sxwmData->currentTags;

	// Create the frame window and reparent the new window within in
	frameClient(client);

	XMapWindow(display, window);

	// Add the client to the linked list
	if (workspace->clients) (workspace->clients)->previous = client;
	client->next = workspace->clients;
	client->previous = NULL;

	workspace->clients = client;
	workspace->clientCount++;

	for (int i = 0; i < sizeof(int)*8; i++) {
		int t = 1 << i;
		if (client->tags & t) sxwmData->windowCounts[i]++;
	}

	// Do the same for the focus list
	if (workspace->focused) (workspace->focused)->focusPrevious = client;
	client->focusNext = workspace->focused;
	client->focusPrevious = NULL;
	workspace->focused = client;
	sxwmData->focusedWindow = client->window;

	// Tile the windows
	tile();
}