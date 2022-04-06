#include "tiling.h"
#include "../../sxwm.h"
#include "../../wm.h"

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
	struct Client *client = malloc(sizeof(struct Client));
	client->frame    = 0;
	client->window   = window;
	client->floating = 0;
	client->tags     = sxwmData->currentTags;

	// Create the frame window and reparent the new window within in
	struct FrameSizePosHint l = {
		10, 10, 10, 10, 600, 600, 600, 600, FRAME_FS | FRAME_FP
	};
	workspace->fd->create(workspace, client, &l);

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

/*
 * Remove a client from this workspace.
 *
 * This could be used when a window is destroyed, or when a window is moved to
 * another workspace. Remove the frame around the client, and remove it from
 * the two linked lists.
 */
void removeClient(struct Workspace *workspace, struct Client *client)
{
	workspace->fd->destroy(workspace, client);

	/* Remove this client from the first linked list */
	if (client->previous) client->previous->next = client->next;
	else workspace->clients = client->next;
	if (client->next) client->next->previous = client->previous;

	/* Remove this client from the second linked list */
	if (client->focusPrevious) client->focusPrevious->focusNext = client->focusNext;
	else workspace->focused = client->focusNext;
	if (client->focusNext) client->focusNext->focusPrevious = client->focusPrevious;

	// Refresh then shared memory variables
	for (int i = 0; i < sizeof(int)*8; i++) {
		int t = 1 << i;
		if (client->tags & t) sxwmData->windowCounts[i]--;
	}
	if (workspace->focused) sxwmData->focusedWindow = workspace->focused->window;
	else sxwmData->focusedWindow = 0;

	workspace->clientCount--;

	// Free this client structure
	free(client);

	// Tile the windows
	tile();
}