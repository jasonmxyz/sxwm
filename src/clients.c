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
	if (client->frame) {
		die("TODO");
	}

	// Create the frame window with the same attributes as the client window
	XWindowAttributes attrs;
	XGetWindowAttributes(display, client->window, &attrs);

	client->frame = XCreateSimpleWindow(display, root,
										 attrs.x, attrs.y,
										 attrs.width, attrs.height,
										 settings.borderWidth, settings.borderColor, 0xffffff);
	XSelectInput(display, client->frame, SubstructureRedirectMask | SubstructureNotifyMask);

	// Reparent the client window within the frame and map the frame
	XReparentWindow(display, client->window, client->frame, 0, 0);
	XMapWindow(display, client->frame);

	// Grab the required keys from the frame
	XGrabKey(display, XKeysymToKeycode(display, XK_space), Mod4Mask, client->window, 1, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_c), Mod4Mask | ShiftMask, client->window, 1, GrabModeAsync, GrabModeAsync);
	XGrabButton(display, Button1, Mod4Mask, client->window, 1, ButtonPressMask | ButtonReleaseMask | ButtonMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(display, Button3, Mod4Mask, client->window, 1, ButtonPressMask | ButtonReleaseMask | ButtonMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	
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

// Get the frame window given the client window
Window getClientFrame(Window window) {
	Monitor* monitor = monitorList;
	Client* current = monitor->clients;
	while (current != NULL) {
		if (current->window == window) {
			return current->frame;
		}
		current = current->next;
	}
	return (Window)NULL;
}

// Removes a client from the list given the client window
void removeClient(Window window) {
	Monitor* monitor = monitorList;
	for (Client* c = monitor->clients; c != NULL; c = c->next) {
		if (c->window == window) {
			if (c->previous != NULL) (c->previous)->next = c->next;
			else monitor->clients = c->next;
			if (c->next != NULL) (c->next)->previous = c->previous;
			monitor->clientCount--;
			for (int i = 0; i < sizeof(int)*8; i++) {
				int t = 1 << i;
				if (c->tags & t) sxwmData->windowCounts[i]--;
			}

			// Update the focus list
			if (c->focusPrevious != NULL) (c->focusPrevious)->focusNext = c->focusNext;
			else monitor->focused = c->focusNext;
			if (c->focusNext != NULL) (c->focusNext)->focusPrevious = c->focusPrevious;
			if (monitor->focused) sxwmData->focusedWindow = monitor->focused->window;
			else sxwmData->focusedWindow = 0;

			free(c);
			return;
		}
	}
}

// Return the Client* associated with a window
Client* getClientByWindow(Window window) {
	Monitor* monitor = monitorList;
	Client* current = monitor->clients;
	while (current != NULL) {
		if (current->window == window) {
			return current;
		}
		current = current->next;
	}
	return NULL;
}