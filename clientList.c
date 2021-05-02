#include "clientList.h"

Client* clients;

// Add a client to the front of the list
void addClient(Client* client) {
	(*client).next = clients;
	clients = client;
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