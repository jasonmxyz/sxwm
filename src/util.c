#include "util.h"
#include "sxwm.h"

#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

/*
 * Print a formatted message to the standard error stream prepended with the
 * basename of the executable and exit.
 *
 * Reads the name of the execuatble from /proc/self/cmdline.
 * Detaches the process from the shared memory segment before exiting.
 */
void die(const char *fmt, ...)
{
	int fd = open("/proc/self/cmdline", O_RDONLY);
	if (fd < 0) {
		return;
	}

	/* Read the cmdline up to PATH_MAX characters, then use basename(3) to
	 * extract the basename. */
	char buf[PATH_MAX];
	if (read(fd, buf, PATH_MAX) < 0) {
		goto diel_close;
	}
	char *bn = basename(buf);

	/* Initialise the argument list. */
	va_list args;
	va_start(args, fmt);

	/* Use the format string and arguments to generate a message with
	 * vsprintf(3) storing a pointer to the result in in *message. The
	 * first call is used to determine the size of the string, this could
	 * be replaced with a call to vasprintf(3), but this is not a standard
	 * C/POSIX function. */
	char useless;
	int messagelen = vsnprintf(&useless, 1, fmt, args);
	if (messagelen < 0) {
		goto diel_va_end;
	}
	va_end(args);
	va_start(args, fmt);
	char *message = malloc(messagelen + 1);
	if (message < 0) {
		goto diel_va_end;
	}
	if (vsprintf(message, fmt, args) < 0) {
		goto diel_free_message;
	}

	/* Allocate enough space to store the complete message and copy it
	 * in. We don't need a null terminator as we use write(3). */
	int basenamelen = strlen(bn);
	char *output = malloc(basenamelen + 2 + messagelen + 1);
	if (output < 0) {
		goto diel_free_message;
	}

	memcpy(output, bn, basenamelen);
	output[basenamelen] = ':';
	output[basenamelen + 1] = ' ';
	memcpy(output + basenamelen + 2, message, messagelen);
	output[basenamelen + 2 + messagelen] = '\n';

	/* Give all of the output in a single call to write(3). */
	write(2, output, basenamelen + 2 + messagelen + 1);

	/* Clean up allcoated memory and close cmdline file. This probably
	 * isn't needed as we're just about to exit, but it can't hurt. */
	free(output);
diel_free_message:
	free(message);
diel_va_end:
	va_end(args);
diel_close:
	close(fd);
	
	if (shmName != NULL) shm_unlink(shmName);

	exit(1);
}