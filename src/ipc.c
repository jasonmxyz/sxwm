#include "util.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int sockfd;
char socketname[108];

/*
 * Create a UNIX socket at the given part, or choose a default path based on
 * the display name. Stores the file descriptor in the global variable sockfd.
 *
 * On success returns 0.
 * On failure returns -1.
 */
int createSocket(const char *path) {
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
		/* Choose a path based on the X11 display name. */
		char *displayName = getenv("DISPLAY");
		if (!displayName) {
			errorf("Could not determine X11 display name.");
			return -1;
		}
		int displayNameLen = strlen(displayName);
		if (displayNameLen > 93) {
			errorf("Display name is too long.");
			return -1;
		}

		memcpy(sock.sun_path, "/tmp/sxwm", 9);
		memcpy(sock.sun_path + 9, displayName, displayNameLen);
		strcpy(sock.sun_path + 9 + displayNameLen, ".sock");
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
	int fd = socket(AF_UNIX, SOCK_DGRAM, 0);
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