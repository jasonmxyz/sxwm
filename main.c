#include "global.h"
#include "clientList.h"

#include <X11/Xlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

char** g_argv;    // Copy of argv to use in other functions
Display* display; // The X display to connected to
Window root;      // The root window of this display
bool existsWM;    // Is there already a window manager running on this display

extern Client* clients;

int detectWM(Display* display, XErrorEvent* e);
int errorHandler(Display* display, XErrorEvent* e);

int main(int argc, char** argv) {
	g_argv = argv; // Preserve argv.
	clients = NULL; // Initialise client list to null.

	// Attempt to open the default display.
	display = XOpenDisplay(NULL);
	if (display == NULL)
		die("Could not connect to X display.");

	root = XDefaultRootWindow(display);

	// Attempt to select substructure redirection on the root window.
	// An error should only occur when there is already a window manager running, so the error
	// handler will set existWM to true.
	existsWM = false;
	XSetErrorHandler(detectWM);
	XSelectInput(display, root, SubstructureRedirectMask | SubstructureNotifyMask);
	XSync(display, false);

	// If we detected another window manager, we must stop.
	if (existsWM) {
		XCloseDisplay(display);
		die("There is already a window manager running on this display.");
	}

	// Set the final error handler
	XSetErrorHandler(errorHandler);

	// Infinite message loop
	while (true) {
		XEvent e;
		XNextEvent(display, &e);
		
		switch (e.type) {
			case ConfigureRequest: {
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
				break;
			}
			case MapRequest: {
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
				printf("Completed Map Request\n");
				#endif
			}
			default:
				#ifdef VERBOSE
				printf("Ignored event\n");
				#endif
		}
	}

	XCloseDisplay(display);

	return 0;
}

// An error handler which does nothing.
int errorHandler(Display* display, XErrorEvent* e) {
	#ifdef VERBOSE
	printf("Handling error.\n");
	#endif
	return 0;
}

// An error handler which sets the boolean existsWM if an error occurs while selecting substructure
// redirection.
int detectWM(Display* display, XErrorEvent* e) {
	existsWM = true;
	return 0;
}