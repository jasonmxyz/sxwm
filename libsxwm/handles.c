#include "handles.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

/* This structure allows us to make a linked list of our handles. */
struct sxwm_handle_ll {
	struct sxwm_handle_ll *prev;
	struct sxwm_handle_ll *next;
	union sxwm_handle entry;
};

/* The first entry in our list of handles. */
static struct sxwm_handle_ll *SXWMHandleLL = NULL;

/* The highest assigned handle identifier. */
static SXWMHandle SXWMHighestHandle = (SXWMHandle)0;

/*
 * Returns a pointer to the sxwm_handle union associated with the given
 * SXWMHandle identifier.
 *
 * Performs a linear search through the linked list, if the given handle does
 * not correspond to an entry, return NULL.
 *
 * On success returns a pointer to the sxwm_handle structure corresponding to
 *                    the given SXWMHandle.
 * On failure returns NULL and sets errno to indicate the error.
 * Errors:
 *  EINVAL: The specified handle identifier did not correspond to an entry in
 *          the linked list.
 */
const union sxwm_handle *SXWMGetHandle(const SXWMHandle handle)
{
	const struct sxwm_handle_ll *item;

	for (item = SXWMHandleLL; item; item = item->next) {
		if (item->entry.any.handle == handle) {
			return &(item->entry);
		}
	}

	errno = EINVAL;
	return NULL;
}

/*
 * Makes a copy of the given handle union, assigns a unique handle identifier
 * to the structure and stores it in a linked list. The structure can later be
 * retrieved with a call to SXWMGetHandle
 *
 * A global SXWMHandle stores the largest handle assigned so far. This value is
 * incremented and assigned to the given sxwm_handle union.
 *
 * On success returns a pointer to the sxwm_handle within the linked list.
 * On failure returns NULL and sets errno to indicate the error.
 * Errors:
 *  errno may be set by malloc(3).
 */
const union sxwm_handle *SXWMRegisterHandle(const union sxwm_handle const *handle)
{
	struct sxwm_handle_ll *newHandle;

	newHandle = malloc(sizeof(struct sxwm_handle_ll));
	if (!newHandle) {
		/* malloc(3) sets errno. */
		return NULL;
	}

	union sxwm_handle *newHandleEntry = &(newHandle->entry);

	/* Copy the sxwm_handle, then replace the actual handle. */
	memcpy(newHandleEntry, handle, sizeof(union sxwm_handle));
	newHandleEntry->any.handle = ++SXWMHighestHandle;

	newHandle->prev = NULL;
	newHandle->next = SXWMHandleLL;
	SXWMHandleLL = newHandle;

	return &(newHandle->entry);
}

/*
 * Remove the item corresponding to the given SXWMHandle from the linked list.
 *
 * On success returns 0.
 * On failure returns -1 and sets errno to indicate the error.
 * Errors:
 *  EINVAL: The specified handle identifier did not correspond to an entry in
 *          the linked list.
 */
int SXWMRemoveHandle(const SXWMHandle handle)
{
	struct sxwm_handle_ll *item;

	for (item = SXWMHandleLL; item; item = item->next) {
		if (item->entry.any.handle == handle) {
			break;
		}
	}

	/* If the linked list entry was not found, it can not be removed. */
	if (!item) {
		errno = EINVAL;
		return -1;
	}

	/* Update the list. */
	if (!item->prev) {
		SXWMHandleLL = item->next;
	} else {
		item->prev->next = item->next;
	}
	if (item->next) {
		item->next->prev = item->prev;
	}

	/* Free the handle linked list item. */
	free(item);

	return 0;
}