#include "clientList.h"
#include "util.h"
#include <stdlib.h>

Client* clients = NULL;
int clientCount = 0;

// Add a client to the front of the list
void addClient(Client* client) {
	(*client).next = clients;
	clients = client;
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
	// If the list is empty, then we cannot remove anything
	if (clients == NULL) die("There are no clients windows, one can't be removed.");

	// If the client is at the front of the list
	if (clients->window == window) {
		Client* temp = clients->next;
		free(clients);
		clients = temp;
		clientCount--;
		return;
	}

	// Otherwise, continue until we reach the end of the list
	Client* current = clients;
	while (current->next != NULL) {
		if ((current->next)->window == window) {
			Client* next = (current->next)->next;
			free(current->next);
			current->next = next;
			clientCount--;
			return;
		}
	}

	// Die if something goes wrong
	die("The given window could not be found in the list.");
}