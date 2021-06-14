#include "clients.h"
#include "util.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>

extern Display* display;
Monitor* monitor = NULL;
Client* clients = NULL;
int clientCount = 0;

// Add a client to the front of the list
void addClient(Client* client) {
	client->next = clients;
	clients = client;
	if(clients->next != NULL) (clients->next)->previous = clients;
	clientCount++;
}

// Get the frame window given the client window
Window getClientFrame(Window window) {
	Client* current = clients;
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
	for (Client* c = clients; c != NULL; c = c->next) {
		if (c->window == window) {
			if (c->previous != NULL) (c->previous)->next = c->next;
			else clients = c->next;
			if (c->next != NULL) (c->next)->previous = c->previous;
			clientCount--;
			free(c);
			return;
		}
	}
}

// Return the Client* associated with a window
Client* getClientByWindow(Window window) {
	Client* current = clients;
	while (current != NULL) {
		if (current->window == window) {
			return current;
		}
		current = current->next;
	}
	return NULL;
}

void getMonitors() {
	if (monitor != NULL) free(monitor);
	monitor = malloc(sizeof(Monitor));

	int s = DefaultScreen(display);
	monitor->width = DisplayWidth(display, s);
	monitor->height = DisplayHeight(display, s);
}