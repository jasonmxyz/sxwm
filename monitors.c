#include "monitors.h"

#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>

extern Display* display;

Monitor* monitor = NULL;

void getMonitors() {
	if (monitor != NULL) free(monitor);
	monitor = malloc(sizeof(Monitor));

	int s = DefaultScreen(display);
	monitor->width = DisplayWidth(display, s);
	monitor->height = DisplayHeight(display, s);

	#ifdef VERBOSE
	printf("Detected screen size: %dx%d\n", monitor->width, monitor->height);
	#endif
}