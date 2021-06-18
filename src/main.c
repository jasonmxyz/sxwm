#include "util.h"
#include "clients.h"
#include "settings.h"
#include "shared.h"

#include <X11/Xlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <X11/keysym.h>
#include <string.h>
#include <getopt.h>
#include <libgen.h>

Display* display;        // The X display to connected to
Window root;             // The root window of this display

Shared* shared = NULL;
Monitor** monitorList = NULL;

char** g_argv = NULL;

extern KeyCombo* rootKeyCombos;
extern CmdQueue* commandQueue;

extern void createBar();
extern void handle(XEvent e);
extern int errorHandler(Display* display, XErrorEvent* e);
extern void readSettings(char* path);
extern void runCmd(char* cmd);

int detectWM(Display* display, XErrorEvent* e);
void getMonitors();

int main(int argc, char** argv) {
	// Preserve argv
	g_argv = argv;

	// Create the shared memory region and get the location of some data structures within it.
	createSharedMemory();

	// Allocate a structure to share between this process and the bar
	shared = smalloc(sizeof(Shared));
	if (shared == NULL) die("Could not allocate memory.");
	monitorList = &(shared->monitor);

	// Populate the shared structure with some important information to share.
	shared->currentTag = 1;
	shared->bar = (Window)NULL;
	shared->running = true;

	// Get the command line options
	char* configFile = NULL;
	struct option options[3] = {{"help", no_argument, 0, 'h'},
								{"config", required_argument, 0, 'c'},
								{0, 0, 0, 0}};
	int opt;
	opterr = 0; // Stop getopt from printing errors.
	// Look through argv for the above options
	while ((opt = getopt_long(argc, argv, ":hc:", options, NULL)) != -1) {
		switch (opt) {
			// If the -h or --help option was given then print a usage statement and exit
			case 'h':
				printf("Usage: %s [--help] [--config <FILE>]\n", basename(argv[0]));
				exit(0);
			// If a config file is specified, remember the char* in argv
			case 'c':
				configFile = optarg;
				break;
			// If an unrecognised option was given
			case '?':
				die("Unrecognised option %s", argv[optind-1]);
			case ':':
				die("Missing argument for %s", argv[optind-1]);
			default:
				die("Unknown error while parsing arguments.");
		}
	}

	// Attempt to open the default display.
	display = XOpenDisplay(NULL);
	if (display == NULL)
		die("Could not connect to X display.");

	// Load the settings into the relevant data structures
	readSettings(configFile);

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

	return 0;
}

// An error handler which dies if there is a window manager already present.
int detectWM(Display* display, XErrorEvent* e) {
	die("There is already a window manager running on this display.");
	return 1;
}

// Get information about the display and store it in the monitor structure.
void getMonitors() {
	*monitorList = malloc(sizeof(Monitor));

	int s = DefaultScreen(display);
	Monitor* monitor = *monitorList;
	monitor->width = DisplayWidth(display, s);
	monitor->height = DisplayHeight(display, s);
	monitor->clients = NULL;
	monitor->clientCount = 0;
}