#include "clients.h"
#include "settings.h"
#include "util.h"
#include "sxwm.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

extern Display* display;
extern Monitor* monitorList;
extern Window root;

extern Settings settings;

// Create a frame window for a given client
void frameClient(Client* client) {
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
	XGrabKey(display, XKeysymToKeycode(display, XK_space), Mod4Mask, client->window, 1, GrabModeAsync, GrabModeAsync);
	XGrabButton(display, Button1, Mod4Mask, client->window, 1, ButtonPressMask | ButtonReleaseMask | ButtonMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(display, Button3, Mod4Mask, client->window, 1, ButtonPressMask | ButtonReleaseMask | ButtonMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	
}

// Destroy the frame around a client
void destroyFrame(Client* client) {
	// Unmap the frame window and reparent the client window under the root.
	XUnmapWindow(display, client->frame);
	XReparentWindow(display, client->window, root, 0, 0);

	// Destroy the frame
	XDestroyWindow(display, client->frame);
}

// Add a client to the front of the list
void addClient(Client* client) {
	Monitor* monitor = monitorList;
	if (monitor->clients != NULL) (monitor->clients)->previous = client;
	client->next = monitor->clients;
	client->previous = NULL;
	monitor->clients = client;
	monitor->clientCount++;
	for (int i = 0; i < sizeof(int)*8; i++) {
		int t = 1 << i;
		if (client->tags & t) sxwmData->windowCounts[i]++;
	}

	// Do the same for the focus list
	if (monitor->focused != NULL) (monitor->focused)->focusPrevious = client;
	client->focusNext = monitor->focused;
	client->focusPrevious = NULL;
	monitor->focused = client;
	sxwmData->focusedWindow = client->window;
}

// Get a client structure given the frame or client window
Client* getClient(Window window, int isFrame) {
	Monitor* monitor = monitorList;
	Client* front = monitor->clients;

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
void removeClient(Client* client) {
	Monitor* monitor = monitorList;

	// Remove this client from the first linked list.
	if (client->previous) client->previous->next = client->next;
	else monitor->clients = client->next;
	if (client->next) client->next->previous = client->previous;

	// Remove this client from the second linked list.
	if (client->focusPrevious) client->focusPrevious->focusNext = client->focusNext;
	else monitor->focused = client->focusNext;
	if (client->focusNext) client->focusNext->focusPrevious = client->focusPrevious;

	// Refresh then shared memory variables
	for (int i = 0; i < sizeof(int)*8; i++) {
		int t = 1 << i;
		if (client->tags & t) sxwmData->windowCounts[i]--;
	}
	if (monitor->focused) sxwmData->focusedWindow = monitor->focused->window;
	else sxwmData->focusedWindow = 0;

	monitor->clientCount--;

	// Free this client structure
	free(client);
}

// Dump client data to stdout for debugging
#ifdef VERBOSE
void dumpClients() {
    Monitor* monitor = monitorList;

	printf("Variable | Stacking Order | Focus Order\n\n");

	Client* list1 = monitor->clients;
	Client* list2 = monitor->focused;
	while (list1 && list2) {
		printf("Address:  %10d    | %10d\n", list1 ? list1 : 0, list2 ? list2 : 0);
		printf("Window:   %10d    | %10d\n", list1 ? list1->window : 0, list2 ? list2->window : 0);
		printf("Frame:    %10d    | %10d\n", list1 ? list1->frame : 0, list2 ? list2->frame : 0);
		printf("Next:     %10d    | %10d\n", list1 ? list1->next : 0, list2 ? list2->next : 0);
		printf("Prevous:  %10d    | %10d\n", list1 ? list1->previous : 0, list2 ? list2->previous : 0);
		printf("fNext:    %10d    | %10d\n", list1 ? list1->focusNext : 0, list2 ? list2->focusNext : 0);
		printf("fPrevous: %10d    | %10d\n", list1 ? list1->focusPrevious : 0, list2 ? list2->focusPrevious : 0);
		printf("\n");
		list1 = list1 ? list1->next : 0;
		list2 = list2 ? list2->focusNext : 0;
	}
}
#endif