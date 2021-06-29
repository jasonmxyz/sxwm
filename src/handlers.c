#include "util.h"
#include "clients.h"
#include "settings.h"
#include "sxwm.h"

#include <stdio.h>
#include <X11/Xlib.h>
#include <stdlib.h>
#include <X11/Xutil.h>
#include <string.h>

extern Display* display;
extern Window root;

extern void keyPress(XEvent e);
extern void buttonPress(XEvent e);
extern void motionNotify(XEvent e);

void configureRequest(XEvent e);
void mapRequest(XEvent e);
void unmapNotify(XEvent e);
int nothingHandler(Display* display, XErrorEvent* e);
int errorHandler(Display* display, XErrorEvent* e);

extern void tile();

void handle(XEvent e) {
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
		default:
			break;
	}
}

void configureRequest(XEvent e) {
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
	Client* client = getClient(e.xconfigurerequest.window, 0);
	if (client && client->frame)
		XConfigureWindow(display, client->frame, e.xconfigurerequest.value_mask, &c);

	// Allow the window to be configured
	XConfigureWindow(display, e.xconfigurerequest.window, e.xconfigurerequest.value_mask, &c);
}

void mapRequest(XEvent e) {
	// Copy the attributes of the window to be created, so that we can create a new one
	// to frame it
	XWindowAttributes attrs;
	XGetWindowAttributes(display, e.xmaprequest.window, &attrs);

	// If this is the bar, or another window with the override_redirect flag,
	// then let it map itself however it likes.
	if (attrs.override_redirect) {
		XMapWindow(display, e.xmaprequest.window);
		return;
	}

	// Create the client
	Client* client = malloc(sizeof(Client));
	client->frame    = 0;
	client->window   = e.xmaprequest.window;
	client->floating = 0;
	client->tags     = sxwmData->currentTags;

	// Create the frame window and reparent the new window within in
	frameClient(client);

	// Add the window to the save set and map it
	XAddToSaveSet(display, e.xmaprequest.window);
	XMapWindow(display, e.xmaprequest.window);

	// Add the client to the linked list
	addClient(client);

	// Send an expose message to the bar
	if (sxwmData->barWindow != 0) {
		XEvent event;
		event.type = Expose;
		XSendEvent(display, sxwmData->barWindow, 1, NoEventMask, &event);
	}

	// Tile the windows
	tile();
}

void unmapNotify(XEvent e) {
	// We only need to act if this window is a client window with a frame that
	// we can destroy.
	Client* client = getClient(e.xunmap.window, 0);
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

// An error handler which does nothing
int nothingHandler(Display* display, XErrorEvent* e) { return 0; }

// An error handler which does nothing.
int errorHandler(Display* display, XErrorEvent* e) {
	return 0;
}