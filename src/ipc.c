#include "monitors.h"
#include "util.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sxwm.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int sockfd;
char socketname[108];

int clientEcho(int clientfd, struct sxwm_header *header, void *data);
int clientGetMonitors(int clientfd, struct sxwm_header *header, void *data);

int (*clientHandler[SXWM_MAX+1])(int, struct sxwm_header*, void*) = {
	[SXWM_ECHO] = clientEcho,
	[SXWM_GETMONITORS] = clientGetMonitors
};

extern struct Monitor *monitorList;

/*
 * Create a UNIX socket at the given path, or choose a default path based on
 * the display name. Stores the file descriptor in the global variable sockfd.
 *
 * On success returns 0.
 * On failure returns -1.
 */
int createSocket(const char *path)
{
	struct sockaddr_un sock;
	sock.sun_family = AF_UNIX;

	if (path) {
		/* The sockaddr_un.sun_path is 108 bytes long, we can therefore
		 * only fit 107 bytes. */
		int len = strlen(path);
		if (len > 107) {
			errorf("Given pathname is too long.");
			return -1;
		}

		strcpy(sock.sun_path, path);
	} else {
		if (SXWMDefaultSocketPath(sock.sun_path) < 0) {
			return -1;
		}
	}

	/* We also preserve the socker name in the global buffer 'socketname'
	 * so it can be unlinked in cleanup. */
	memcpy(socketname, sock.sun_path, 108);

	/* If the socket already exists, remove it. */
	if (unlink(sock.sun_path) < 0 && errno != ENOENT) {
		errorf("Could not remove socket: %s", strerror(errno));
		return -1;
	}

	/* Create and bind the socket. */
	int fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) {
		errorf("Could not create socket: %s", strerror(errno));
		return -1;
	}
	if (bind(fd, (struct sockaddr*) &sock, sizeof(struct sockaddr_un))) {
		errorf("Could not bind socket: %s", strerror(errno));
		return -1;
	}

	sockfd = fd;

	return 0;
}

/*
 * Recieve and (possibly) respond to a message from a client over the given
 * socket.
 *
 * Performs two recv(2) calls to get the header, and then the payload before
 * calling the function which corresponds to the sxwm_header.type value.
 */
void handleClientRequest(int clientfd)
{
	struct sxwm_header header;
	void *data = SXWMRecieve(clientfd, &header);

	if (!data && header.size != 0) {
		errorf("Error recieving data: %s", strerror(errno));
		/* Maybe we should disconnect from this client. */
		return;
	}

	if (header.type > SXWM_MAX) {
		errorf("Invalid message type (%d)", header.type);
		/* Maybe we should disconnect from this client. */
		free(data);
		return;
	}

	int (*function)(int, struct sxwm_header*, void*) = clientHandler[header.type];
	if (function) {
		function(clientfd, &header, data);
	}

	free(data);
}

/*
 * Respond to a SXWM_ECHO message by sending the given data back to the client.
 *
 * On success returns 0.
 * On failure returns -1.
 */
int clientEcho(int clientfd, struct sxwm_header *header, void *data)
{
	return SXWMSendSeq(clientfd, header->type, header->size, data, header->seq);
}

#include <stdio.h>

/*
 * Responds to a SXWM_GETMONITORS message, sending the relevant data structures
 * to the client.
 *
 * On success returns 0.
 * On failure returns -1.
 */
int clientGetMonitors(int clientfd, struct sxwm_header *header, void *data)
{
	/* Calculate the size of the data to send. */
	int size = sizeof(struct sxwm_monitor_spec);
	int nmonitors = 0;
	for (struct Monitor *m = monitorList; m; m = m->next) {
		nmonitors++;
		size += strlen(m->name);
	}
	size += nmonitors * (sizeof(struct sxwm_monitor_spec_item) + 1);

	/* Allocate the message data. */
	void *message = malloc(size);
	if (!message) {
		return -1;
	}

	printf("size: %d\n", size);

	struct sxwm_monitor_spec *monitorSpec = message;
	char *names = (char*)&(monitorSpec->monitors[nmonitors]);

	/* Fill in the structures. */
	monitorSpec->nmonitors = nmonitors;
	int i = 0;
	for (struct Monitor *m = monitorList; m; m = m->next) {
		struct sxwm_monitor_spec_item *item = &(monitorSpec->monitors[i++]);

		/* Copy the name into the free space at the end, advance the
		 * 'names' pointer and add it to the structure. */
		int len = strlen(m->name) + 1;
		memcpy(names, m->name, len);
		item->nameoffset = (void*)names - message;
		names += len;

		item->id = m->id;
		item->x = m->x;
		item->y = m->y;
		item->width = m->width;
		item->height = m->height;
	}

	int ret = SXWMSendSeq(clientfd, header->type, size, message, header->seq);
	free(message);
	return ret;
}