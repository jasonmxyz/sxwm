#include "clients.h"
#include "util.h"
#include "shared.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>

extern Display* display;

// Add a client to the front of the list
void addClient(Client* client) {
	Monitor* monitor = *monitorList;
	if (monitor->clients != NULL) (monitor->clients)->previous = client;
	client->next = monitor->clients;
	monitor->clients = client;
	monitor->clientCount++;
}

// Get the frame window given the client window
Window getClientFrame(Window window) {
	Monitor* monitor = *monitorList;
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
	Monitor* monitor = *monitorList;
	for (Client* c = monitor->clients; c != NULL; c = c->next) {
		if (c->window == window) {
			if (c->previous != NULL) (c->previous)->next = c->next;
			else monitor->clients = c->next;
			if (c->next != NULL) (c->next)->previous = c->previous;
			monitor->clientCount--;
			sfree(c);
			return;
		}
	}
}

// Return the Client* associated with a window
Client* getClientByWindow(Window window) {
	Monitor* monitor = *monitorList;
	Client* current = monitor->clients;
	while (current != NULL) {
		if (current->window == window) {
			return current;
		}
		current = current->next;
	}
	return NULL;
}