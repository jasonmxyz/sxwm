#include "clients.h"
#include "monitors.h"
#include "settings.h"
#include "sxwm.h"
#include "workspaces/workspaces.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

extern Display* display;
extern Window root;
extern struct Monitor *selectedMonitor;

extern void keyPress(XEvent e);
extern void buttonPress(XEvent e);
extern void motionNotify(XEvent e);

void configureRequest(XEvent e);
void mapRequest(XEvent e);
void unmapNotify(XEvent e);
void enterNotify(XEvent e);
int nothingHandler(Display* display, XErrorEvent* e);
int errorHandler(Display* display, XErrorEvent* e);

extern void tile();

void handle(XEvent e)
{
	switch (e.type) {
		case ConfigureRequest:
			configureRequest(e);
			break;
		case MapRequest:
			mapRequest(e);
			break;
		case UnmapNotify:
			unmapNotify(e);
			break;
		case KeyPress:
			keyPress(e);
			break;
		case ButtonPress:
			buttonPress(e);
			break;
		case MotionNotify:
			motionNotify(e);
			break;
		case EnterNotify:
			enterNotify(e);
			break;
		default:
			break;
	}
}

void configureRequest(XEvent e)
{
	// Can be recieved multiple times while an application is running, so best not to
	// do much.

	// A structure to indicate no changes being done to the window.
	XWindowChanges c;
	c.x = e.xconfigurerequest.x;
	c.y = e.xconfigurerequest.y;
	c.width = e.xconfigurerequest.width;
	c.height = e.xconfigurerequest.height;
	c.border_width = e.xconfigurerequest.border_width;
	c.sibling = e.xconfigurerequest.above;
	c.stack_mode = e.xconfigurerequest.detail;

	// The window which frames this one also needs to be reconfigured (if it exists)
	struct Client *client = getClient(e.xconfigurerequest.window, 0);
	if (client && client->frame)
		XConfigureWindow(display, client->frame, e.xconfigurerequest.value_mask, &c);

	// Allow the window to be configured
	XConfigureWindow(display, e.xconfigurerequest.window, e.xconfigurerequest.value_mask, &c);
}

/*
 * We recieve a MapRequest event when a child of some window calls XMapWindow,
 * but we have select the substructure redirection flag on its parent. We have
 * only selected substructure redirection on the root window.

 * We give the window to the currently selected workspace. It should not be
 * possible for a window with the override_redirect flag to appear here. We add
 * windows to the save set, although it is never used by this program.
 *
 * The function at WorkspaceDescription.newClient is responsible for adding the
 * client to its linked list and creating a reparenting it within some frame if
 * necessary.
 */
void mapRequest(XEvent e)
{
	struct Workspace *workspace = selectedMonitor->workspaces;

	XAddToSaveSet(display, e.xmaprequest.window);

	workspace->wd->newClient(workspace, e.xmaprequest.window);

	// Send an expose message to the bar
	if (sxwmData->barWindow != 0) {
		XEvent event;
		event.type = Expose;
		XSendEvent(display, sxwmData->barWindow, 1, NoEventMask, &event);
	}
}

void unmapNotify(XEvent e)
{
	// We only need to act if this window is a client window with a frame that
	// we can destroy.
	struct Client *client = getClient(e.xunmap.window, 0);
	if (!client) return;

	// Destroy the frame around this client
	destroyFrame(client);

	// Remove this window from the save set
	XRemoveFromSaveSet(display, client->window);

	// Remove this client from the linked lists.
	removeClient(client);

	// Send an expose message to the bar
	if (sxwmData->barWindow != 0) {
		XEvent event;
		event.type = Expose;
		XSendEvent(display, sxwmData->barWindow, 1, NoEventMask, &event);
	}

	// Tile the windows
	tile();
}

void enterNotify(XEvent e)
{
	// Determine the client being entered
	struct Client *client = getClient(e.xcrossing.window, 1);
	struct Workspace *workspace = selectedMonitor->workspaces;

	// If this is not a client, then just focus the root window
	if (!client) {
		XSetInputFocus(display, root, RevertToPointerRoot, CurrentTime);
		return;
	}

	// If this is a client, then bring it to the front of the focus list
	// Remove it from the focus list
	if (client->focusPrevious) client->focusPrevious->focusNext = client->focusNext;
	else workspace->focused = client->focusNext;
	if (client->focusNext) client->focusNext->focusPrevious = client->focusPrevious;
	// Add it back at the start
	client->focusPrevious = NULL;
	client->focusNext = workspace->focused;
	if (workspace->focused) workspace->focused->focusPrevious = client;
	workspace->focused = client;

	// Maybe change the border around this window and the previously focused one
	// to indicate focus (TODO?)

	// Focus the client window
	// DWM does it like this, and DWM is cool so...
	XSetInputFocus(display, client->window, RevertToPointerRoot, CurrentTime);
	XEvent event;
	event.type = ClientMessage;
	event.xclient.window = client->window;
	event.xclient.message_type = XInternAtom(display, "WM_PROTOCOLS", 0);
	event.xclient.format = 32;
	event.xclient.data.l[0] = XInternAtom(display, "WM_TAKE_FOCUS", 0);
	event.xclient.data.l[1] = CurrentTime;
	XSendEvent(display, client->window, 0, NoEventMask, &event);

	// Update the shared memory
	sxwmData->focusedWindow = client->window;

	// Send an expose message to the bar
	if (sxwmData->barWindow != 0) {
		XEvent event;
		event.type = Expose;
		XSendEvent(display, sxwmData->barWindow, 1, NoEventMask, &event);
	}
}

// An error handler which does nothing
int nothingHandler(Display* display, XErrorEvent* e)
{
	return 0;
}

// An error handler which does nothing.
int errorHandler(Display* display, XErrorEvent* e)
{
	return 0;
}