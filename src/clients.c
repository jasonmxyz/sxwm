#include "clients.h"
#include "monitors.h"
#include "settings.h"
#include "sxwm.h"
#include "workspaces/workspaces.h"

#include <stdio.h>
#include <stdlib.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>

extern Display* display;
extern Window root;
extern struct Monitor *selectedMonitor;

extern Settings settings;

// Create a frame window for a given client
void frameClient(struct Client *client)
{
	// If the client has a frame window, then remove it.
	if (client->frame) destroyFrame(client);

	// Create the frame window with the same attributes as the client window
	XWindowAttributes attrs;
	XGetWindowAttributes(display, client->window, &attrs);

	client->frame = XCreateSimpleWindow(display, root,
		attrs.x, attrs.y,
		attrs.width, attrs.height,
		settings.borderWidth, settings.borderColor, 0xffffff);
	XSelectInput(display, client->frame, SubstructureRedirectMask | SubstructureNotifyMask | EnterWindowMask);

	// Reparent the client window within the frame and map the frame
	XReparentWindow(display, client->window, client->frame, 0, 0);
	XMapWindow(display, client->frame);

	// Grab the required keys from the frame
	XGrabButton(display, Button1, Mod4Mask, client->window, 1, ButtonPressMask | ButtonReleaseMask | ButtonMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(display, Button3, Mod4Mask, client->window, 1, ButtonPressMask | ButtonReleaseMask | ButtonMotionMask, GrabModeAsync, GrabModeAsync, None, None);
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
struct Client *getClient(Window window, int isFrame)
{
	struct Workspace *workspace = selectedMonitor->workspaces;
	struct Client *front = workspace->clients;

	// If we are searching with a frame window
	if (isFrame) {
		for (; front != NULL; front = front->next)
			if (front->frame == window)
				return front;
		return NULL;
	}

	// If we are searching with a client window
	for (; front != NULL; front = front->next)
		if (front->window == window)
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