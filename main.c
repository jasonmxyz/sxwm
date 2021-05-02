#include "util.h"
#include "clientList.h"
#include "handlers.h"
#include "monitors.h"

#include <X11/Xlib.h>
#include <stdio.h>
#include <stdbool.h>

char** g_argv;    // Copy of argv to use in other functions
Display* display; // The X display to connected to
Window root;      // The root window of this display

int detectWM(Display* display, XErrorEvent* e);
int errorHandler(Display* display, XErrorEvent* e);

int main(int argc, char** argv) {
	g_argv = argv; // Preserve argv.

	// Attempt to open the default display.
	display = XOpenDisplay(NULL);
	if (display == NULL)
		die("Could not connect to X display.");

	root = XDefaultRootWindow(display);

	// Attempt to select substructure redirection on the root window.
	// An error should only occur when there is already a window manager running, causing death.
	XSetErrorHandler(detectWM);
	XSelectInput(display, root, SubstructureRedirectMask | SubstructureNotifyMask);
	XSync(display, false);

	// Set the final error handler
	XSetErrorHandler(errorHandler);

	// Populate the screen structure with the geometry of the display
	getMonitors();

	// Infinite message loop
	while (true) {
		XEvent e;
		XNextEvent(display, &e);
		
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
			default:
				#ifdef VERBOSE
				printf("Ignored event\n");
				#endif
				break;
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

// An error handler which dies if there is a window manager already present.
int detectWM(Display* display, XErrorEvent* e) {
	die("There is already a window manager running on this display.");
	return 1;
}