#include "util.h"
#include "clientList.h"
#include "handlers.h"
#include "monitors.h"
#include "tile.h"
#include "input.h"
#include "bar.h"

#include <X11/Xlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <X11/keysym.h>

char** g_argv;         // Copy of argv to use in other functions
Display* display;      // The X display to connected to
Window root;           // The root window of this display
void* settings = NULL; // The settings structure
bool running;

int barHeight = 30;
int borderWidth = 1;
int currentTag = 1;

int detectWM(Display* display, XErrorEvent* e);

int main(int argc, char** argv) {
	g_argv = argv; // Preserve argv.
	
	createBar();

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

	// Create the settings structure with the default settings
	settings = malloc(sizeof(tileSettings));
	((tileSettings*)settings)->masterCount = 1;
	((tileSettings*)settings)->masterRatio = 0.6;
	((tileSettings*)settings)->gapSize = 10;

	XGrabKey(display, XKeysymToKeycode(display, XK_q), Mod4Mask | ShiftMask, root, true, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_p), Mod4Mask | ShiftMask, root, true, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_Return), Mod4Mask | ShiftMask, root, true, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_1), Mod4Mask, root, true, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_2), Mod4Mask, root, true, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_3), Mod4Mask, root, true, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_4), Mod4Mask, root, true, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_5), Mod4Mask, root, true, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_6), Mod4Mask, root, true, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_7), Mod4Mask, root, true, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_8), Mod4Mask, root, true, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_9), Mod4Mask, root, true, GrabModeAsync, GrabModeAsync);
	
	// Infinite message loop
	running = true;
	while (running) {
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

	XCloseDisplay(display);

	return 0;
}

// An error handler which dies if there is a window manager already present.
int detectWM(Display* display, XErrorEvent* e) {
	die("There is already a window manager running on this display.");
	return 1;
}