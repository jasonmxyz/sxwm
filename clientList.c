#include "clientList.h"

Client* clients;

// Add a client to the front of the list
void addClient(Client* client) {
	(*client).next = clients;
	clients = client;
}