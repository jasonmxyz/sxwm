#include "monitors.h"
#include "util.h"
#include "wm.h"

#include <stdlib.h>
#include <string.h>
#include <X11/extensions/Xrandr.h>
#include <X11/Xlib.h>

extern Display *display;
extern Window root;
extern struct WorkspaceDescription *defaultWorkspace;
extern struct FrameDescription *defaultFrame;

/* The fist monitor in the list. */
struct Monitor *monitorList = NULL;

static struct Monitor *getSingleMonitor();
static struct Monitor *getRandRMonitorList();
static void freeBlankList(struct Monitor *list);
static int uniqueCounter = 1;
static int uniqueWorkspaceCounter = 1;

/*
 * Generate a linked list of monitor structure representing the physical
 * displays attached to the display server.
 *
 * Detect if the RandR extension is enabled on the default screen. If it is,
 * then use it to determine the position and dimensions of all screens on the
 * display. Otherwise use Xlib macros and create the linked list of one monitor
 * with the dimensions of the default screen.
 *
 * Existing memory should be used if possible, but workspaces will be moved
 * into new structures if necessary. (TODO).
 *
 * On success returns the number of monitors in the list.
 * On failure returns -1, writing any error messages to stderr.
 */
int detectMonitors()
{
	int ret; /* Unused variable to store XQueryExtension results in. */
	struct Monitor *newList;

	if (!XQueryExtension(display, "RANDR", &ret, &ret, &ret)) {
		newList = getSingleMonitor();
	} else {
		newList = getRandRMonitorList();
	}

	if (!newList) {
		return -1;
	}

	if (monitorList) {
		/* TODO: Replace existing monitor list with new one. */
		errorf("Not yet implimented.");
		return -1;
	} else {
		/* The first time we generate the list, we assign the default
		 * workspace to each monitor. */
		for (struct Monitor *m = newList; m; m = m->next) {
			struct Workspace *newWorkspace;
			newWorkspace = malloc(sizeof(struct Workspace));
			if (!newWorkspace) {
				/* TODO: Free only new monitor memory so we do
				 * not have to die here. */
				die("%s", "Could not allocate memory.");
				return -1;
			}

			newWorkspace->x = m->x;
			newWorkspace->y = m->y;
			newWorkspace->width = m->width;
			newWorkspace->height = m->height;
			newWorkspace->id = uniqueWorkspaceCounter++;
			newWorkspace->clients = NULL;
			newWorkspace->focused = NULL;
			newWorkspace->clientCount = 0;
			newWorkspace->fd = defaultFrame;
			newWorkspace->wd = defaultWorkspace;
			newWorkspace->prev = NULL;
			newWorkspace->next = NULL;

			m->workspaces = newWorkspace;
		}
	}

	monitorList = newList;

	int i = 0;
	for (struct Monitor *m = monitorList; m; m = m->next) {
		i++;
	}

	return i;
}

/*
 * Generate a linked list containing a single monitor structure with the
 * dimensions of the default screen.
 *
 * We use this when the RandR extension is not available.
 *
 * On success returns a pointer to a monitor structure with a default name and
 *            the dimensions of the screen.
 * On failure returns null, writing any error messages to stderr.
 */
static struct Monitor *getSingleMonitor()
{
	int screen = XDefaultScreen(display);

	/* Allocate enough space for the structure. */
	struct Monitor *m = malloc(sizeof(struct Monitor));
	if (!m) {
		errorf("%s", "Could not allocate memory.");
		return NULL;
	}

	/* Allocate enough space for a placeholder name. */
	m->name = malloc(8);
	if (!(m->name)) {
		free(m);
		errorf("%s", "Could not allocate memory.");
		return NULL;
	}

	/* Populate the structure with values relevant to the screen. */
	memcpy(m->name, "DEFAULT", 8);
	m->x = 0;
	m->y = 0;
	m->width = XDisplayWidth(display, screen);
	m->height = XDisplayHeight(display, screen);
	m->id = uniqueCounter++;
	m->prev = NULL;
	m->next = NULL;
	m->workspaces = NULL;

	return m;
}

/*
 * Generate a linked list of monitor structures with their corresponsding
 * names, dimensions, and positions as known by RandR.
 *
 * Consider only 'active' monitors by setting the 'get_active' flag in
 * XRRGetMonitors.
 *
 * On success returns a pointer to the first monitor in a linked list.
 * On failure returns null, writing any error messages to stderr.
 */
static struct Monitor *getRandRMonitorList()
{
	int nmonitors = 0;
	XRRMonitorInfo *mons = XRRGetMonitors(display, root, 1, &nmonitors);

	/* Documentation for XRRGetMonitors is hard to find, it probably isn't
	 * meant to be used, we assume that nmonitors will return 0 or negative
	 * if something goes wrong, or mons will be NULL. If something goes
	 * wrong we will fallback to the list with a single monitor. */
	if (!mons || nmonitors <= 0) {
		errorf("%s", "RandR error, falling back to single monitor...");
		return getSingleMonitor();
	}

	struct Monitor *m = NULL;
	struct Monitor *last = NULL;
	for (int i = 0; i < nmonitors; i++) {
		struct Monitor *new = malloc(sizeof(struct Monitor));
		if (!new) {
			errorf("%s", "Could not allocate memory.");
			freeBlankList(m);
			return NULL;
		}

		/* Find the name of the display, and add it to the structure in
		 * new dynamic memory. */
		char *name = XGetAtomName(display, mons[i].name);
		int length = strlen(name) + 1;
		new->name = malloc(length);
		if (!(new->name)) {
			errorf("%s", "Could not allocate memory.");
			freeBlankList(m);
			return NULL;
		}
		memcpy(new->name, name, length);

		/* Populate the structure with other relevant values. */
		new->x = mons[i].x;
		new->y = mons[i].y;
		new->width = mons[i].width;
		new->height = mons[i].height;
		new->id = uniqueCounter++;
		new->prev = last;
		new->next = NULL;
		new->workspaces = NULL;

		/* Update the previous linked list item. */
		if (last) {
			last->next = new;
		}
		if (!m) {
			m = new;
		}
		last = new;
	}

	return m;
}

/*
 * Free the memory associated with a linked list of monitors with no extra
 * data.
 *
 * Used in the event of an error in the getRandRMonitorList function to free
 * the list before calling the fallback function and in detectMonitors to free
 * unused monitor structures.
 */
static void freeBlankList(struct Monitor *list)
{
	struct Monitor *front = list;
	while (front) {
		struct Monitor *current = front;
		front = current->next;

		if (current->name) {
			free(current->name);
		}

		/* Free all the workspaces for this monitor. */
		struct Workspace *workspaces = current->workspaces;
		while (workspaces) {
			struct Workspace *currentWorkspace = workspaces;
			workspaces = currentWorkspace->next;

			free(currentWorkspace);
		}

		free(current);
	}
}