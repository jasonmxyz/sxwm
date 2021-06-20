#include "util.h"

#include <X11/Xlib.h>

char** g_argv;

int main(int argc, char** argv) {
	DEBUG("Start of bar process.");
	g_argv = argv;

	DEBUG("Opening display.");
	Display* display = XOpenDisplay(NULL);
	Window root = XDefaultRootWindow(display);
	
	int screen = DefaultScreen(display);
	int width = DisplayWidth(display, screen);
	DEBUG("Screen width: %d.", width);

	DEBUG("Closing Display.");
	XCloseDisplay(display);

	return 0;
}