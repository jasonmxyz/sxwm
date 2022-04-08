/*
 * In this file we define functions to query the status of the monitors SXWM is
 * using.
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sxwm.h>
#include <sxwm/msg.h>

/*
 * Gets data about the monitor configuration used by SXWM.
 * 
 * Send a SXWM_GETMONITORS message and awaits a response, allocates a block of
 * memory to store an array of `struct sxwm_monitor` as well as workspaces IDs
 * and null terminated strings. The returned block should later be passed to a
 * call to free.
 *
 * On success returns a pointer to a `struct sxwm_monitor` and returns the
 *                    number of detected monitors in retN.
 * On failure returns NULL and sets errno to indicate the error.
 * Errors:
 *  EINVAL: The given handle does not match a socket object.
 *  EIO: The response from the window manager did not match the request.
 *  errno may be set by malloc(3), send(2), and recv(2).
 */
struct sxwm_monitor *SXWMGetMonitors(SXWMHandle handle, int *retN)
{
	int socket = SXWMSocketFD(handle);
	if (socket < 0) {
		/* Sets errno to EINVAL. */
		return NULL;
	}

	/* Send the message and recieve the response from the next message. */
	int seq;
	if ((seq = SXWMSend(socket, SXWM_GETMONITORS, 0, NULL)) == -1) {
		/* Sets errno. */
		return NULL;
	}

	struct sxwm_header respHeader;
	void *data = SXWMRecieve(socket, &respHeader);
	if (!data) {
		/* Sets errno. */
		return NULL;
	}

	/* The next message we recieve should be the response to the one we
	 * just sent. */
	if (respHeader.type != SXWM_GETMONITORS || respHeader.seq != seq) {
		errno = EIO;
		free(data);
		return NULL;
	}

	/* Create pointers to the sections in the message. */
	struct sxwm_message_getmonitors_header *header;
	struct sxwm_message_getmonitors_monitordata *monitorData;
	uint32_t *msgworkspaces;
	char *msgnames;

	header = data;
	monitorData = data + header->monitorDataOffset;
	msgworkspaces = data + header->workspaceIDsOffset;
	msgnames = data + header->namesOffset;
	int extrasize = respHeader.size - header->workspaceIDsOffset;

	/* Determine the size of the result data block. */
	int size = respHeader.size
		 + ( ( sizeof(struct sxwm_monitor)
		       - sizeof(struct sxwm_message_getmonitors_monitordata) )
		     * header->nMonitors
		   );
	
	/* Allocate the new block and copy in the workspace and name data. */
	void *final = malloc(size);
	if (!final) {
		/* Sets errno. */
		return NULL;
	}

	struct sxwm_monitor *monitors;
	uint32_t *workspaces;
	char *names;

	monitors = final;
	workspaces = final + (sizeof(struct sxwm_monitor) * header->nMonitors);
	names = (void*)workspaces + header->namesOffset - header->workspaceIDsOffset;

	memcpy(workspaces, msgworkspaces, extrasize);

	/* Copy the data from the message. */
	for (int i = 0; i < header->nMonitors; i++) {
		monitors[i].id = monitorData[i].id;
		monitors[i].name = names + monitorData[i].nameOffset;
		monitors[i].x = monitorData[i].x;
		monitors[i].y = monitorData[i].y;
		monitors[i].width = monitorData[i].width;
		monitors[i].height = monitorData[i].height;
		monitors[i].nWorkspaces = monitorData[i].nWorkspaces;
		monitors[i].selectedWorkspace = monitorData[i].selectedWorkspace;
		monitors[i].workspaces = workspaces + monitorData[i].workspacesOffset;
	}

	/* Free the original message and return the new block. */
	*retN = header->nMonitors;
	free(data);
	return monitors;
}