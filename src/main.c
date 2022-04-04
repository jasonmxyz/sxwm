#include "clients.h"
#include "monitors.h"
#include "settings.h"
#include "sxwm.h"
#include "util.h"

#include <fcntl.h>
#include <getopt.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>

Display* display;        // The X display to connected to
Window root;             // The root window of this display

struct Options {
	int present_help : 1;
	int present_config : 1;
	char *configFile;
} options;

extern int running;

/* The currently selected monitor. */
extern struct Monitor *monitorList;
struct Monitor *selectedMonitor;

extern KeyCombo* rootKeyCombos;
extern CmdQueue* commandQueue;
extern BarSettings barSettings;

extern void handle(XEvent e);
extern int errorHandler(Display* display, XErrorEvent* e);
extern void readSettings(char* path);
extern void startProgram(char* cmd, int newSession);
extern int detectMonitors();

int detectWM(Display* display, XErrorEvent* e);

static void parseCmdLine(int argc, char **argv);

int main(int argc, char** argv)
{
	/* Read the command line arguments. */
	parseCmdLine(argc, argv);

	// Attempt to open the default display.
	display = XOpenDisplay(NULL);
	if (display == NULL)
		die("Could not connect to X display.");

	// Load the settings into the relevant data structures
	readSettings(options.configFile);

	root = XDefaultRootWindow(display);

	// Attempt to select substructure redirection on the root window.
	// An error should only occur when there is already a window manager running, causing death.
	XSetErrorHandler(detectWM);
	XSelectInput(display, root, SubstructureRedirectMask | SubstructureNotifyMask);
	XSync(display, 0);

	// Set the final error handler
	XSetErrorHandler(errorHandler);

	// Create the shared memory
	int shmFd = createShared(1);
	if (ftruncate(shmFd, sizeof(SXWMData)) == -1)
		die("Could not create shared memory.");
	mapShared(1);
	memset(sxwmData, 0, sizeof(SXWMData));
	// Set the value of the variables.
	sxwmData->currentTags = 1;
	// Copy settings into shared memory for the bar to use
	memcpy(&(sxwmData->barSettings), &barSettings, sizeof(BarSettings));

	/* Generate the monitor list. */
	if (detectMonitors() < 0) {
		die("Failure detecting display setup.");
	}
	selectedMonitor = monitorList;

	// Run all of the commands from the command queue in new processes, and free up the memory used
	// by the queue and the command strings inside it.
	CmdQueue* front = commandQueue;
	while (front != NULL) {
		startProgram(front->cmd, front->newSession);
		free(front->cmd);
		CmdQueue* next = front->next;
		free(front);
		front = next;
	}

	// Grab all the requested keys on the root window
	for (KeyCombo* front = rootKeyCombos; front != NULL; front = front->next)
		XGrabKey(display, front->keycode, front->modifiers, root, 1, GrabModeAsync, GrabModeAsync);
	
	// Infinite message loop
	while (running) {
		XEvent e;
		XNextEvent(display, &e);
		
		handle(e);
	}

	XCloseDisplay(display);
	shm_unlink(shmName);

	return 0;
}

// An error handler which dies if there is a window manager already present.
int detectWM(Display* display, XErrorEvent* e)
{
	die("There is already a window manager running on this display.");
	return 1;
}

/*
 * Print a usage message to the standard output stream.
 *
 * The argument `name` should be a pointer to a null terminated string
 * containing the path to the executable as given to exec.
 */
static void printusage(char *name)
{
	char *bn = basename(name);

	printf("Usage: %s%s", bn, " [--help] [--config <FILE>]\n");
}

/*
 * Parse command line arguments using typical syntax and populate the option
 * structure with the found options.
 *
 * Use the getopt function provided by libc to ensure typical command line
 * behavior.
 *
 *  Calls die (from util.c) on error.
 */
static void parseCmdLine(int argc, char **argv)
{
	/* Zero-fill the options structure. */
	memset(&options, 0, sizeof(options));

	/* The options we can detect with getopt. */
	struct option getopts[3] = {
		{"help", no_argument, 0, 'h'},
		{"config", required_argument, 0, 'c'},
		{0, 0, 0, 0}
	};
	
	int opt;
	opterr = 0; /* Supress errors from getopt. */

	while ((opt = getopt_long(argc, argv, ":hc:", getopts, NULL)) != -1) {
	switch (opt) {
	case 'h': /* -h or --help */
		printusage(argv[0]);
		options.present_help = 1;
		break;
	case 'c': /* -c or --config */
		options.present_config = 1;
		options.configFile = optarg;
		break;
	case '?': /* Unrecognised option */
		die("%s%s", "Unrecognised option ", argv[optind - 1]);
		return;
	case ':': /* Option given without an argument. */
		die("%s%s", "Missing argument for option ", argv[optind - 1]);
		return;
	default: /* Something else. */
		die("%s", "Unknown error while parsing arguments.");
		return;
	}
	}
}