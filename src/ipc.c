#include "monitors.h"
#include "util.h"
#include "wm.h"

#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <sxwm.h>
#include <sxwm/msg.h>
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
int handleClientRequest(int clientfd)
{
	struct sxwm_header header;
	void *data = SXWMRecieve(clientfd, &header);

	if (!data && header.size != 0) {
		errorf("Error recieving data: %s", strerror(errno));
		return -1;
	}

	if (header.type > SXWM_MAX) {
		errorf("Invalid message type (%d)", header.type);
		free(data);
		return -1;
	}

	int (*function)(int, struct sxwm_header*, void*) = clientHandler[header.type];
	int ret;
	if (function) {
		ret = function(clientfd, &header, data);
	}

	free(data);
	return ret;
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

/*
 * Responds to a SXWM_GETMONITORS message, sending the relevant data structures
 * to the client. The message is described in docs/message/getmonitors.md.
 *
 * On success returns 0.
 * On failure returns -1 and sets errno to indicate the error.
 * Errors:
 *  errno may be set by malloc(3) or SXWMSendSeq.
 */
int clientGetMonitors(int clientfd, struct sxwm_header *header, void *data)
{
	/* Calculate the size of the message to send. */
	int nMonitors = 0;
	int nWorkspaces = 0;
	int namesLength = 0;
	for (struct Monitor *m = monitorList; m; m = m->next) {
		nMonitors++;

		for (struct Workspace *w = m->workspaces; w; w = w->next) {
			nWorkspaces++;
		}

		namesLength += strlen(m->name) + 1;
	}
	int size = sizeof(struct sxwm_message_getmonitors_header)
		 + ( sizeof(struct sxwm_message_getmonitors_monitordata)
		     * nMonitors )
		 + ( sizeof(uint32_t) * nWorkspaces )
		 + namesLength;
	
	/* Allocate space for the message. */
	void *message = malloc(size);
	if (!message) {
		/* malloc(3) sets errno. */
		return -1;
	}

	/* Create some useful pointers to the sections in the message. */
	struct sxwm_message_getmonitors_header *msgHeader;
	struct sxwm_message_getmonitors_monitordata *monitorData;
	uint32_t *workspaces;
	char *names;

	msgHeader = message;
	monitorData = (void*)msgHeader
		    + sizeof(struct sxwm_message_getmonitors_header);
	workspaces = (void*)monitorData
		   + ( sizeof(struct sxwm_message_getmonitors_monitordata)
		       * nMonitors );
	names = (void*)workspaces
	      + ( sizeof(uint32_t) * nWorkspaces );
	
	/* Allows us to index in the arrays. */
	int monitorN = 0;
	int workspacesN = 0;
	char *nextName = names;

	/* Go through the monitor list again. */
	for (struct Monitor *m = monitorList; m; m = m->next) {
		monitorData[monitorN].id = m->id;
		monitorData[monitorN].nameOffset = nextName - names;
		monitorData[monitorN].x = m->x;
		monitorData[monitorN].y = m->y;
		monitorData[monitorN].width = m->width;
		monitorData[monitorN].height = m->height;
		monitorData[monitorN].workspacesOffset = workspacesN;
		monitorData[monitorN].selectedWorkspace = m->workspaces->id;

		int localNworkspaces = 0;
		for (struct Workspace *w = m->workspaces; w; w = w->next) {
			workspaces[workspacesN++] = w->id;
			localNworkspaces++;
		}
		monitorData[monitorN].nWorkspaces = localNworkspaces;

		int nameLen = strlen(m->name) + 1;
		memcpy(nextName, m->name, nameLen);
		nextName += nameLen;

		monitorN++;
	}
	
	/* Populate the header fields. */
	msgHeader->nMonitors = monitorN;
	msgHeader->monitorDataOffset = (void*)monitorData - message;
	msgHeader->workspaceIDsOffset = (void*)workspaces - message;
	msgHeader->namesOffset = (void*)names - message;

	/* Send the message with the same header type and sequence number. */
	int ret = SXWMSendSeq(clientfd, header->type, size, message, header->seq);
	free(message);
	return ret;
}