#include "util.h"
#include "clientList.h"
#include "monitors.h"

#include <X11/Xlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <X11/keysym.h>
#include <sys/shm.h>
#include <sys/stat.h>

char** g_argv;         // Copy of argv to use in other functions
Display* display;      // The X display to connected to
Window root;           // The root window of this display
bool running;

Shared* shared; // A shared memory segment
int sid; // The id of the segment

extern void createBar();
extern void handle(XEvent e);
extern int errorHandler(Display* display, XErrorEvent* e);

int detectWM(Display* display, XErrorEvent* e);

int main(int argc, char** argv) {
	g_argv = argv; // Preserve argv.

	// Create the shared memory segment with read and write permissions on a new private segment.
	int sid = shmget(IPC_PRIVATE, sizeof(Shared), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
	shared = shmat(sid, NULL, 0);

	shared->currentTag = 1;
	shared->bar = (Window)NULL;
	
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
		
		handle(e);
	}

	XCloseDisplay(display);

	// Detatch from and destroy the shared memory segment
	shmdt(shared);
    shmctl(sid, IPC_RMID, 0);

	return 0;
}

// An error handler which dies if there is a window manager already present.
int detectWM(Display* display, XErrorEvent* e) {
	die("There is already a window manager running on this display.");
	return 1;
}