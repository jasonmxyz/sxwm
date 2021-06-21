#include "clients.h"
#include "util.h"
#include "sxwm.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>

extern Display* display;
extern Monitor* monitorList;

// Add a client to the front of the list
void addClient(Client* client) {
	Monitor* monitor = monitorList;
	if (monitor->clients != NULL) (monitor->clients)->previous = client;
	client->next = monitor->clients;
	monitor->clients = client;
	monitor->clientCount++;
	for (int i = 0; i < sizeof(int)*8; i++) {
		int t = 1 << i;
		if (client->tags & t) sxwmData->windowCounts[i]++;
	}

	// Do the same for the focus list
	if (monitor->focused != NULL) (monitor->focused)->focusPrevious = client;
	client->focusNext = monitor->focused;
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