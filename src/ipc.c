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
 * Determine the default location of the SXWM socket. Write the path in the
 * destination buffer up to 108 bytes.
 *
 * We combine the name of the default display (from the DISPLAY environment
 * variable) with constant strings to make /tmp/sxwm:X.sock.
 *
 * On success returns 0.
 * On failure returns -1.
 */
int defaultSocketPath(char *dest)
{
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

	memcpy(dest, "/tmp/sxwm", 9);
	memcpy(dest + 9, displayName, displayNameLen);
	strcpy(dest + 9 + displayNameLen, ".sock");

	return 0;
}

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
		if (defaultSocketPath(sock.sun_path) < 0) {
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
 * Create a socket and connect to SXWM at the given socket path. If no path is
 * given, then a default path is generated based on the name of the display.
 *
 * On success returns the file descriptor associated with the connection.
 * On failure returns -1.
 */
int connectSocket(const char *path)
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
		if (defaultSocketPath(sock.sun_path) < 0) {
			return -1;
		}
	}

	/* Create and connect the socket. */
	int fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) {
		errorf("Could not create socket: %s", strerror(errno));
		return -1;
	}
	if (connect(fd, (struct sockaddr *)&sock, sizeof(struct sockaddr_un))) {
		errorf("Could not connect: %s", strerror(errno));
		return -1;
	}

	return fd;
}