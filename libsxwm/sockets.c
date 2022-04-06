/*
 * This file contains functions relating to the socket connection between a
 * client and the SXWM window manager.
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

 /*
 * Determine the default location of the SXWM socket. Write the path in the
 * destination buffer up to 108 bytes.
 *
 * We combine the name of the default display (from the DISPLAY environment
 * variable) with constant strings to make /tmp/sxwm:X.sock.
 *
 * On success returns the length of the string written in the buffer,
 *                    including the terminating null byte.
 * On failure returns -1 and sets errno to indicate the error.
 *
 * Errors:
 *  ENAMETOOLONG: The calculated name would not fit in a 108 byte buffer.
 *  EINVAL: The environment variable DISPLAY is not set.
 */
int SXWMDefaultSocketPath(char *buffer)
{
	char *displayName = getenv("DISPLAY");
	if (!displayName) {
		errno = EINVAL;
		return -1;
	}
	int displayNameLen = strlen(displayName);
	if (displayNameLen > 93) {
		errno = ENAMETOOLONG;
		return -1;
	}

	memcpy(buffer, "/tmp/sxwm", 9);
	memcpy(buffer + 9, displayName, displayNameLen);
	strcpy(buffer + 9 + displayNameLen, ".sock");

	return 15 + displayNameLen;
}