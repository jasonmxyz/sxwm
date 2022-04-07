/*
 * In this file we define functions to query the status of the monitors SXWM is
 * using.
 */

#include <errno.h>
#include <stdlib.h>
#include <sxwm.h>

/*
 * Get data about all of the monitors in use by SXWM.
 *
 * Sends a request, then immediately awaits a response. Checks that the
 * recieved data was sent in response to our message. The returned pointer must
 * be passed in a call to free.
 *
 * On success returns a pointer to a sxwm_monitor_spec structure.
 * On failure returns null and errno is set to indicate the error.
 */
struct sxwm_monitor_spec *SXWMGetMonitors(int socket)
{
	int seq;
	if ((seq = SXWMSend(socket, SXWM_GETMONITORS, 0, NULL)) == -1) {
		return NULL;
	}

	struct sxwm_header header;
	void *data = SXWMRecieve(socket, &header);
	if (!data) {
		return NULL;
	}

	if (header.type != SXWM_GETMONITORS || header.seq != seq) {
		errno = EIO;
		free(data);
		return NULL;
	}

	return data;
}