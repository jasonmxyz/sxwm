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
 * basename of the executable given a variable argument list. Analogous to the
 * vprintf(3) family.
 *
 * Reads the name of the execuatble from /proc/self/cmdline.
 */
static void verrorf(const char *fmt, va_list args)
{
	/* Copy the va_list because I don't know how to 'restart' it, and we
	 * need it twice. Both of these lists have been 'started', but we will
	 * only stop the second one. */
	va_list args2;
	va_copy(args2, args);

	int fd = open("/proc/self/cmdline", O_RDONLY);
	if (fd < 0) {
		return;
	}

	/* Read the cmdline up to PATH_MAX characters, then use basename(3) to
	 * extract the basename. */
	char buf[PATH_MAX];
	if (read(fd, buf, PATH_MAX) < 0) {
		goto verrorf_close;
	}
	char *bn = basename(buf);

	/* Determine the length of the formatted message with vsnprintf(3),
	 * allowing it to only write 1 byte. */
	char useless;
	int messagelen = vsnprintf(&useless, 1, fmt, args);
	if (messagelen < 0) {
		goto verrorf_close;
	}

	/* Allocate enough space to store the complete message and copy it
	 * in. We don't need a null terminator as we use write(3). */
	int basenamelen = strlen(bn);
	char *output = malloc(basenamelen + 2 + messagelen + 1);
	if (output < 0) {
		goto verrorf_close;
	}

	memcpy(output, bn, basenamelen);
	output[basenamelen] = ':';
	output[basenamelen + 1] = ' ';
	vsprintf(output + basenamelen + 2, fmt, args2);
	output[basenamelen + 2 + messagelen] = '\n';

	/* Give all of the output in a single call to write(3). */
	write(2, output, basenamelen + 2 + messagelen + 1);

	/* Clean up allcoated memory and close cmdline file. */
	free(output);
verrorf_close:
	va_end(args2);
	close(fd);
}

/*
 * Print a formatted message to the standard error stream prepended with the
 * basename of the executable.
 *
 * Reads the name of the execuatble from /proc/self/cmdline.
 */
void errorf(const char *fmt, ...)
{
	/* Initialise the argument list and use it to call errorf. */
	va_list args;
	va_start(args, fmt);
	verrorf(fmt, args);
	va_end(args);
}

/*
 * Print a formatted message to the standard error stream prepended with the
 * basename of the executable and exit.
 *
 * Calls 'verrorf' to print the message.
 * Detaches the process from the shared memory segment before exiting.
 */
void die(const char *fmt, ...)
{
	/* Initialise the argument list and use it to call errorf. */
	va_list args;
	va_start(args, fmt);
	verrorf(fmt, args);
	va_end(args);

	exit(1);
}

/*
 * Things to do before normal process exit. Unlink shared memory object.
 *
 * We register this function with atexit(3), it will be called after 'die',
 * 'exit' or a fatal X11 error is recieved.
 */
void cleanup()
{
	if (shmName != NULL) shm_unlink(shmName);
}