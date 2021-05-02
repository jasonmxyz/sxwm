#include "global.h"
#include "clientList.h"

#include <stdio.h>
#include <X11/Xlib.h>
#include <stdlib.h>

extern Display* display;
extern Window root;

void configureRequest(XEvent e) {
	// Can be recieved multiple times while an application is running, so best not to
	// do much.
	#ifdef VERBOSE
	printf("Recieved Configure Request\n");
	#endif

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
	if (frame != (Window)NULL) {
		XConfigureWindow(display, frame, e.xconfigurerequest.value_mask, &c);
		#ifdef VERBOSE
		printf(" Configured window frame\n");
		#endif
	}

	// Allow the window to be configured
	XConfigureWindow(display, e.xconfigurerequest.window, e.xconfigurerequest.value_mask, &c);

	#ifdef VERBOSE
	printf(" Configured window\n");
	#endif
}

void mapRequest(XEvent e) {
	#ifdef VERBOSE
	printf("Recieved Map Request\n");
	#endif

	// Copy the attributes of the window to be created, so that we can create a new one
	// to frame it
	XWindowAttributes attrs;
	XGetWindowAttributes(display, e.xmaprequest.window, &attrs);

	// Create this window with a border to surround e.xmaprequest.window and save it to
	// the set of all windows for X
	Window framed = XCreateSimpleWindow(display, root, attrs.x, attrs.y, attrs.width, attrs.height, 2, 0x3333ff, 0xffffff);
	XSelectInput(display, framed, SubstructureRedirectMask | SubstructureNotifyMask);
	XAddToSaveSet(display, e.xmaprequest.window);

	// Add a new client structure to the linked list of all clients
	Client* newClient = malloc(sizeof(Client));
	newClient->frame = framed;
	newClient->window = e.xmaprequest.window;
	addClient(newClient);

	// Reparent and map this window as well as its frame
	XReparentWindow(display, e.xmaprequest.window, framed, 0, 0);
	XMapWindow(display, framed);
	XMapWindow(display, e.xmaprequest.window);

	#ifdef VERBOSE
	printf(" Completed\n");
	#endif
}

void unmapNotify(XEvent e) {
	#ifdef VERBOSE
	printf("Recieved Unmap Notification\n");
	#endif
	// When a window has been unmapped, we need to destory the frame and reparent the
	// client under the root window. We don't need to do this for the frame itself.
	Window framed = getClientFrame(e.xunmap.window);
	if (framed == (Window)NULL) {
		#ifdef VERBOSE
		printf(" Not responding because this is a frame\n");
		#endif
		return;
	}

	// Reparent the client, and destroy the frame
	XUnmapWindow(display, framed);
	XReparentWindow(display, e.xunmap.window, root, 0, 0);
	XRemoveFromSaveSet(display, e.xunmap.window);
	XDestroyWindow(display, framed);
	// Remove the client from the linked list
	removeClient(e.xunmap.window);

	#ifdef VERBOSE
	printf(" Removed client\n");
	#endif
}