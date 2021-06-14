#include "util.h"
#include "clientList.h"
#include "monitors.h"
#include "settings.h"

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

Shared* shared; // A shared memory segment
int sid; // The id of the segment
extern KeyCombo* rootKeyCombos;
extern CmdQueue* commandQueue;

extern void createBar();
extern void handle(XEvent e);
extern int errorHandler(Display* display, XErrorEvent* e);
extern void readSettings(char* path);
extern void runCmd(char* cmd);

int detectWM(Display* display, XErrorEvent* e);

int main(int argc, char** argv) {
	g_argv = argv; // Preserve argv.

	// Attempt to open the default display.
	display = XOpenDisplay(NULL);
	if (display == NULL)
		die("Could not connect to X display.");

	// Quick and dirty for now. Check that there is an argument, and pass that to readSettings
	if (argc != 2) die("Incorrect number of arguments.");

	// Load the settings into the relevant data structures
	readSettings(argv[1]);

	// Create the shared memory segment with read and write permissions on a new private segment.
	int sid = shmget(IPC_PRIVATE, sizeof(Shared), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
	shared = shmat(sid, NULL, 0);

	shared->currentTag = 1;
	shared->bar = (Window)NULL;
	shared->running = true;

	root = XDefaultRootWindow(display);

	// Attempt to select substructure redirection on the root window.
	// An error should only occur when there is already a window manager running, causing death.
	XSetErrorHandler(detectWM);
	XSelectInput(display, root, SubstructureRedirectMask | SubstructureNotifyMask);
	XSync(display, false);

	// Set the final error handler
	XSetErrorHandler(errorHandler);

	// Run all of the commands from the command queue in new processes
	CmdQueue* front;
	for (front = commandQueue; front != NULL; front = front->next) {
		runCmd(front->cmd);
	}

	// Populate the screen structure with the geometry of the display
	getMonitors();

	createBar();

	// Grab all the requested keys on the root window
	for (KeyCombo* front = rootKeyCombos; front != NULL; front = front->next)
		XGrabKey(display, front->keycode, front->modifiers, root, true, GrabModeAsync, GrabModeAsync);
	
	// Infinite message loop
	while (shared->running) {
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