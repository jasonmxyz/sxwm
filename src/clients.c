#include "clients.h"
#include "monitors.h"
#include "settings.h"
#include "sxwm.h"
#include "wm.h"

#include <stdio.h>
#include <stdlib.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>

extern Display* display;
extern Window root;
extern struct Monitor *selectedMonitor;
extern struct Monitor *monitorList;

extern Settings settings;

/*
 * Get the Client and Workspace associated with some client window. Returns
 * pointers to the client and workspace structures in the provided fields if
 * they are found.
 *
 * Searches every workspace for a client with the given window - only checks
 * the 'window' field to improve performance. If retClient or retWorkspace is
 * null, it is not accessed.
 *
 * On success returns 0 and stores the pointers in retClient and retWorkspace.
 * On failure returns -1.
 */
int getClientWorkspace(Window window, struct Client **retClient, struct Workspace **retWorkspace)
{
	for (struct Monitor *m = monitorList; m; m = m->next) {
		for (struct Workspace *w = m->workspaces; w; w = w->next) {
			for (struct Client *c = w->clients; c; c = c->next) {
				if (c->window == window) {
					if (retClient) {
						*retClient = c;
					}
					if (retWorkspace) {
						*retWorkspace = w;
					}
					return 0;
				}
			}
		}
	}

	return -1;
}

/*
 * Get the Client and Workspace associated with some window. Returns pointers
 * to the client and workspace structures in the provided fields if they are
 * found.
 *
 * Searches every workspace for a client with the given window. If retClient
 * or retWorkspace is null, it is not accessed.
 *
 * On success returns 0 and stores the pointers in retClient and retWorkspace.
 * On failure returns -1.
 */
int getClientWorkspaceAny(Window window, struct Client **retClient, struct Workspace **retWorkspace)
{
	for (struct Monitor *m = monitorList; m; m = m->next) {
		for (struct Workspace *w = m->workspaces; w; w = w->next) {
			for (struct Client *c = w->clients; c; c = c->next) {
				if (c->window == window) {
					if (retClient) {
						*retClient = c;
					}
					if (retWorkspace) {
						*retWorkspace = w;
					}
					return 0;
				}
				if (c->frame == window) {
					if (retClient) {
						*retClient = c;
					}
					if (retWorkspace) {
						*retWorkspace = w;
					}
					return 0;
				}
			}
		}
	}

	return -1;
}