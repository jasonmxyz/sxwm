/*
 * This file contains functions for sending to and recieving messages from the
 * window manager.
 *
 * Work needs to be done to make these operations more atomic and reliable. If
 * we send or recieve an incomplete message, the window manager, or the client
 * will block indefinitely.
 */

#include <inttypes.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sxwm.h>
#include <sys/socket.h>

uint32_t sxwm_seq = 0;

/*
 * Sends a message to the window manager or client through the given socket
 * with a given sequence number.
 *
 * Allocate memory such that the message can be sent in a single call to
 * send(2).
 *
 * On success returns the sequence number.
 * On failure returns -1 and sets errno to indicate the error.
 * Errors:
 *  ENOMEM: Could not allocate memory to store the message.
 *  EINVAL: The given type is not recognised.
 *  0: The whole message was not sent, this is bad...
 * Other errors may be generated by send(2).
 */
int SXWMSendSeq(int socket, uint32_t type, uint32_t size, void *data, int seq)
{
	if (type > SXWM_MAX || type < 0) {
		errno = EINVAL;
		return -1;
	}

	void *message = malloc(sizeof(struct sxwm_header) + size);
	if (!message) {
		return -1;
	}

	struct sxwm_header *header = message;
	void *payload = message + sizeof(struct sxwm_header);

	header->type = type;
	header->size = size;
	header->seq = seq;

	memcpy(payload, data, size);

	errno = 0;
	ssize_t sent = send(socket, message, sizeof(struct sxwm_header) + size, 0);
	free(message);

	/* If send succeeded, but did not send the whole message, we would be
	 * in trouble. */
	if (sent != sizeof(struct sxwm_header) + size) {
		return -1;
	}

	return (int)(seq);
}

/*
 * Sends a message to the window manager or client through the given socket.
 *
 * Send a single message through the socket containing a header and the payload
 * data. Assigns a unique sequence number to the message.
 *
 * On success returns a unique positive integer representing the sequence
 *                    number of the message, a response to this message will
 *                    include this number.
 * On failure returns -1 and sets errno to indicate the error.
 * Errors:
 *  ENOMEM: Could not allocate memory to store the message.
 *  EINVAL: The given type is not recognised.
 *  0: The whole message was not sent, this is bad...
 * Other errors may be generated by send(2).
 */
int SXWMSend(int socket, uint32_t type, uint32_t size, void *data)
{
	return SXWMSendSeq(socket, type, size, data, ++sxwm_seq);
}

/*
 * Get a message from the window manager or client through the given socket.
 *
 * Performs two calls to recv(2), firstly getting the header, then getting the
 * payload.
 *
 * On success returns a pointer to the contents of the message. This pointer
 *                    must later be passed in a call to free(3). If the size of
 *                    the payload is zero, then the returned value is null, so
 *                    errno should always be tested.
 * On failure returns sets errno to indicate the error.
 * Errors:
 *  ENOMEM: Could not allocate memory to store the message.
 *  ECONNRESET: The connection was closed by the peer.
 *  0: The whole message was not sent, this is bad...
 * Other errors may be generated by recv(2).
 */
void *SXWMRecieve(int socket, struct sxwm_header *header)
{
	errno = 0;
	
	ssize_t got = recv(socket, header, sizeof(struct sxwm_header), 0);
	if (!got) {
		/* recv(2) returns zero when an orderly shutdown of the socket
		 * occured, we return NULL and set errno to ECONNRESET */
		errno = ECONNRESET;
		return NULL;
	}
	/* This would be really bad. */
	if (got != sizeof(struct sxwm_header)) {
		return NULL;
	}

	void *payload = NULL;
	if (header->size > 0) {
		payload = malloc(header->size);
		/* This would also be really bad. */
		if (!payload) {
			return NULL;
		}

		got = recv(socket, payload, header->size, 0);
		if (!got) {
			/* recv(2) returns zero when an orderly shutdown of the socket
			 * occured, we return NULL and set errno to ECONNRESET */
			errno = ECONNRESET;
			return NULL;
		}
		/* This would also be really bad. */
		if (got != header->size) {
			return NULL;
		}
	}

	return payload;
}