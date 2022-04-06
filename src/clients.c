#include "clients.h"
#include "monitors.h"
#include "settings.h"
#include "sxwm.h"
#include "workspaces.h"

#include <stdio.h>
#include <stdlib.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>

extern Display* display;
extern Window root;
extern struct Monitor *selectedMonitor;
extern struct Monitor *monitorList;

extern Settings settings;

/*
 * Get the Client and Workspace associated with some client window. Returns
 * pointer to the client and workspace structures in the provided fields if
 * they are found.
 *
 * Searches every workspace for a client with the given window - only checks
 * the 'window' field to improve performance. If retClient or retWorkspace is
 * null, it is not accessed.
 *
 * On success returns 0 and stores the pointers in retClient and retWorkspace.
 * On failure returns -1.
 */
int getClientWorkspace(Window window, struct Client **retClient, struct Workspace **retWorkspace)
{
	for (struct Monitor *m = monitorList; m; m = m->next) {
		for (struct Workspace *w = m->workspaces; w; w = w->next) {
			for (struct Client *c = w->clients; c; c = c->next) {
				if (c->window == window) {
					if (retClient) {
						*retClient = c;
					}
					if (retWorkspace) {
						*retWorkspace = w;
					}
					return 0;
				}
			}
		}
	}

	return -1;
}

// Destroy the frame around a client
void destroyFrame(struct Client *client)
{
	// Unmap the frame window and reparent the client window under the root.
	XUnmapWindow(display, client->frame);
	XReparentWindow(display, client->window, root, 0, 0);

	// Destroy the frame
	XDestroyWindow(display, client->frame);
}

// Get a client structure given the frame or client window
struct Client *getClient(Window window)
{
	struct Workspace *workspace = selectedMonitor->workspaces;
	struct Client *front = workspace->clients;

	for (; front != NULL; front = front->next)
		if (front->frame == window)
			return front;
	return NULL;
}

// Removes a client its linked lists
void removeClient(struct Client *client)
{
	struct Workspace *workspace = selectedMonitor->workspaces;

	// Remove this client from the first linked list.
	if (client->previous) client->previous->next = client->next;
	else workspace->clients = client->next;
	if (client->next) client->next->previous = client->previous;

	// Remove this client from the second linked list.
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
}