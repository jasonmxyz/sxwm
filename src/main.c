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
#include <string.h>

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
	// Create the shared memory segment with read and write permissions on a new private segment.
	int sid = shmget(IPC_PRIVATE, sizeof(Shared), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
	shared = shmat(sid, NULL, 0);

	// Populate the shared structure with some important information to share. Including a copy of
	// argv.
	shared->currentTag = 1;
	shared->bar = (Window)NULL;
	shared->running = true;
	shared->argc = argc;
	shared->argv = malloc(sizeof(char*) * (argc + 1));
	for (int i = 0; i < argc; i++) {
		// Count the length of the string in argv[i]
		int l = 0;
		while (argv[i][l] != 0) l++;

		// Allocate the string and copy it in.
		shared->argv[i] = malloc(l + 1);
		memcpy(shared->argv[i], argv[i], l + 1);
	}
	shared->argv[argc] = NULL;

	// Attempt to open the default display.
	display = XOpenDisplay(NULL);
	if (display == NULL)
		die("Could not connect to X display.");

	// Quick and dirty for now. Check that there is an argument, and pass that to readSettings
	if (argc != 2) die("Incorrect number of arguments.");

	// Load the settings into the relevant data structures
	readSettings(argv[1]);

	root = XDefaultRootWindow(display);

	// Attempt to select substructure redirection on the root window.
	// An error should only occur when there is already a window manager running, causing death.
	XSetErrorHandler(detectWM);
	XSelectInput(display, root, SubstructureRedirectMask | SubstructureNotifyMask);
	XSync(display, false);

	// Set the final error handler
	XSetErrorHandler(errorHandler);

	// Run all of the commands from the command queue in new processes, and free up the memory used
	// by the queue and the command strings inside it.
	CmdQueue* front = commandQueue;
	while (front != NULL) {
		runCmd(front->cmd);
		free(front->cmd);
		CmdQueue* next = front->next;
		free(front);
		front = next;
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