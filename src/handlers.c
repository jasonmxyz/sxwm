#include "clients.h"
#include "monitors.h"
#include "settings.h"
#include "sxwm.h"
#include "util.h"
#include "wm.h"

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

/*
 * If we recieve a ConfigureRequest event, we will allow the client to
 * configure itself however it likses (within reason).
 * 
 * If the window to be configured is known to us and associated with a client,
 * we only allow it to change its size and position through the
 * FrameDescription.moveresize function. We allow any other window to perform
 * any configuration.
 */
void configureRequest(XEvent e)
{
	struct Client *client;
	struct Workspace *workspace;
	if (getClientWorkspaceAny(e.xconfigurerequest.window, &client, NULL) < 0) {
		errorf("Unrecognised window requesting configuration.");
		XWindowChanges c;
		c.x = e.xconfigurerequest.x;
		c.y = e.xconfigurerequest.y;
		c.width = e.xconfigurerequest.width;
		c.height = e.xconfigurerequest.height;
		c.border_width = e.xconfigurerequest.border_width;
		c.sibling = e.xconfigurerequest.above;
		c.stack_mode = e.xconfigurerequest.detail;
		XConfigureWindow(display, e.xconfigurerequest.window, e.xconfigurerequest.value_mask, &c);
	}

	struct FrameSizePosHint size = {
		e.xconfigurerequest.x,
		e.xconfigurerequest.y,
		e.xconfigurerequest.x,
		e.xconfigurerequest.y,
		e.xconfigurerequest.width,
		e.xconfigurerequest.height,
		e.xconfigurerequest.width,
		e.xconfigurerequest.height,
		0
	};
	if (e.xconfigurerequest.window == client->window) {
		size.mask = FRAME_CP | FRAME_CS;
	} else {
		size.mask = FRAME_FP | FRAME_FS;
	}
	
	workspace->fd->moveresize(workspace, client, &size);
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

/*
 * We recieve an UnmapNotify event when a window is unmapped, and we selected
 * the structure notify mode on it, or the substructure notify mode on its
 * parent. This includes all clients windows, and possibly some windows in the
 * frames, so we must only act when a client window is unmapped.
 *
 * We remove the client from its workspace, which causes its frame to be
 * removed at the same time, then we remove it from the same set and finish.
 */
void unmapNotify(XEvent e)
{
	/* We only care about when client windows are unmapped. */
	struct Client *client;
	struct Workspace *workspace;
	if (getClientWorkspace(e.xunmap.window, &client, &workspace) < 0) {
		return;
	}
	
	workspace->wd->removeClient(workspace, client);
	XRemoveFromSaveSet(display, client->window);

	// Send an expose message to the bar
	if (sxwmData->barWindow != 0) {
		XEvent event;
		event.type = Expose;
		XSendEvent(display, sxwmData->barWindow, 1, NoEventMask, &event);
	}
}

void enterNotify(XEvent e)
{
	/* Find the corresponding client and workspace. */
	struct Client *client;
	struct Workspace *workspace;
	if (getClientWorkspaceAny(e.xcrossing.window, &client, &workspace) < 0) {
		/* If we are not focusing on a client, then do nothing. */
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