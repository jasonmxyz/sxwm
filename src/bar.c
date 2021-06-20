#include "util.h"
#include "sxwm.h"

#include <X11/Xlib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <stdio.h>

char** g_argv;

int main(int argc, char** argv) {
	DEBUG("Start of bar process.");
	g_argv = argv;

	// Create the shared memory
	createShared(0);
	mapShared(0);
	DEBUG("Connected to shared memory.");

	DEBUG("Opening display.");
	Display* display = XOpenDisplay(NULL);
	Window root = XDefaultRootWindow(display);
	
	int screen = DefaultScreen(display);
	int width = DisplayWidth(display, screen);
	DEBUG("Screen width: %d.", width);

	DEBUG("Closing Display.");
	XCloseDisplay(display);

	shm_unlink(shmName);
}
