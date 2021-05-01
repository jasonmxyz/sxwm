#pragma once

typedef struct Client Client;
struct Client {
	Client* next;
};

void addClient(Client* client);