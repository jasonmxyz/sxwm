#include "global.h"
#include "clientList.h"

#include <X11/Xlib.h>
#include <stdio.h>
#include <stdbool.h>

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

		#ifdef VERBOSE
		printf("Recieved event.\n");
		#endif

		switch (e.type) {
			default:
				#ifdef VERBOSE
				printf(" Ignored event");
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