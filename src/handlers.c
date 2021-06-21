#include "util.h"
#include "clients.h"
#include "settings.h"
#include "sxwm.h"

#include <stdio.h>
#include <X11/Xlib.h>
#include <stdlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <string.h>

extern Display* display;
extern Window root;
extern Settings settings;

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
	Window frame = getClientFrame(e.xconfigurerequest.window);
	if (frame != (Window)NULL)
		XConfigureWindow(display, frame, e.xconfigurerequest.value_mask, &c);

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

	// Create this window with a border to surround e.xmaprequest.window and save it to
	// the set of all windows for X
	Window framed = XCreateSimpleWindow(display, root, attrs.x, attrs.y, attrs.width, attrs.height, settings.borderWidth, settings.borderColor, 0xffffff);
	XSelectInput(display, framed, SubstructureRedirectMask | SubstructureNotifyMask);
	XAddToSaveSet(display, e.xmaprequest.window);

	// Add a new client structure to the linked list of all clients
	Client* newClient = calloc(sizeof(Client), 1);
	newClient->frame = framed;
	newClient->window = e.xmaprequest.window;
	newClient->floating = false;
	newClient->tags = sxwmData->currentTags;
	addClient(newClient);

	// Reparent and map this window as well as its frame
	XReparentWindow(display, e.xmaprequest.window, framed, 0, 0);
	XMapWindow(display, framed);
	XGrabKey(display, XKeysymToKeycode(display, XK_space), Mod4Mask, e.xmaprequest.window, true, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_c), Mod4Mask | ShiftMask, e.xmaprequest.window, true, GrabModeAsync, GrabModeAsync);
	XGrabButton(display, Button1, Mod4Mask, e.xmaprequest.window, true, ButtonPressMask | ButtonReleaseMask | ButtonMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(display, Button3, Mod4Mask ,e.xmaprequest.window, true, ButtonPressMask | ButtonReleaseMask | ButtonMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	XMapWindow(display, e.xmaprequest.window);

	// Send an expose message to the bar
	if (sxwmData->barWindow != 0) {
		XEvent event;
		event.type = Expose;
		XSendEvent(display, sxwmData->barWindow, true, NoEventMask, &event);
	}

	// Tile the windows
	tile();
}

void unmapNotify(XEvent e) {
	// When a window has been unmapped, we need to destory the frame and reparent the
	// client under the root window. We don't need to do this for the frame itself.
	Window framed = getClientFrame(e.xunmap.window);
	if (framed == (Window)NULL) return;

	// Reparent the client, and destroy the frame
	XUnmapWindow(display, framed);
	XReparentWindow(display, e.xunmap.window, root, 0, 0);
	XRemoveFromSaveSet(display, e.xunmap.window);
	XDestroyWindow(display, framed);
	// Remove the client from the linked list
	removeClient(e.xunmap.window);

	// Send an expose message to the bar
	if (sxwmData->barWindow != 0) {
		XEvent event;
		event.type = Expose;
		XSendEvent(display, sxwmData->barWindow, true, NoEventMask, &event);
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