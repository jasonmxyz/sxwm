#include "sxwm.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sxwm.h>
#include <sys/mman.h>

GC gc;
Display* display;
int width;

SXWMHandle handle;

void draw();
int drawTags();
void drawTitle(int xpos);

void cleanup()
{
	if (handle) {
		SXWMDisconnectSocket(handle);
	}

	if (display) {
		XCloseDisplay(display);
	}

	shm_unlink(shmName);
}

int main(int argc, char** argv)
{
	atexit(cleanup);

	/* Connect to the window manager. */
	handle = SXWMConnectSocket(NULL);
	if (!handle) {
		printf("Could not connect to window manager.\n");
		return 1;
	}

	struct sxwm_monitor_spec *monitors = SXWMGetMonitors(handle);
	if (!monitors) {
		printf("Could not get monitor data.\n");
		return -1;
	}

	printf("n: %d\n", monitors->nmonitors);
	for (int i = 0; i < monitors->nmonitors; i++) {
		struct sxwm_monitor_spec_item *m = &(monitors->monitors[i]);
		printf("%d:\n", i);
		printf("x: %d\n", m->x);
		printf("y: %d\n", m->y);
		printf("w: %d\n", m->width);
		printf("h: %d\n", m->height);
		printf("nameoffset: %d\n", m->nameoffset);
		printf("name: %s\n", &(((char*)monitors)[m->nameoffset]));
		printf("id: %d\n", m->id);
	}
	
	free(monitors);
	
	// Create the shared memory
	createShared(1);
	mapShared(1);

	display = XOpenDisplay(NULL);
	Window root = XDefaultRootWindow(display);
	
	width = DisplayWidth(display, DefaultScreen(display));

	// Set the window attributes and create the window
	XSetWindowAttributes attrs;
	attrs.override_redirect = True;
	attrs.event_mask = ExposureMask;
	sxwmData->barWindow = XCreateWindow(display, root,
		0, 0,
		width, sxwmData->barSettings.height,
		0,
		CopyFromParent, CopyFromParent, CopyFromParent,
		CWOverrideRedirect | CWEventMask, &attrs);

	XMapWindow(display, sxwmData->barWindow);

	gc = XCreateGC(display, root, 0, NULL);

	XEvent e;
	while(1) {
		XNextEvent(display, &e);
		if (e.type == Expose) {
			draw();
		}
	}
}

void draw() {
	int xpos = drawTags();
	
	XSetForeground(display, gc, sxwmData->barSettings.bgColor1);
	XFillRectangle(display, sxwmData->barWindow, gc, xpos, 0, width-xpos, sxwmData->barSettings.height);

	drawTitle(xpos);
}

int drawTags() {
	int xpos = 0;

	char tag[]= {'1', 0};

	for (int i = 0; i < sizeof(int)*8; i++) {
		// If this tag is selected, or has a window in it, then draw
		int t = 1 << i;
		if ((sxwmData->currentTags & t) || (sxwmData->windowCounts[i] > 0)) {
			// If the tag is selected, then choose color2, otherwise, color1

			if (sxwmData->currentTags & t) XSetForeground(display, gc, sxwmData->barSettings.bgColor2);
			else XSetForeground(display, gc, sxwmData->barSettings.bgColor1);
			XFillRectangle(display, sxwmData->barWindow, gc,
						   xpos, 0,
						   sxwmData->barSettings.height, sxwmData->barSettings.height);
			
			if (sxwmData->currentTags & t) XSetForeground(display, gc, sxwmData->barSettings.fgColor2);
			else XSetForeground(display, gc, sxwmData->barSettings.fgColor1);
			XDrawString(display, sxwmData->barWindow, gc, xpos + 10, 20, tag, 1);

			xpos += sxwmData->barSettings.height;
		}
		tag[0]++;
	}

	return xpos;
}

void drawTitle(int xpos)
{
	XSetForeground(display, gc, sxwmData->barSettings.fgColor1);

	// If there is no focused window, then draw some placeholder text
	if (!sxwmData->focusedWindow) {
		const char* msg = "github.com/jasonmxyz/sxwm";
		XDrawString(display, sxwmData->barWindow, gc, xpos + 10, 20, msg, strlen(msg));
		return;
	}

	// Get the title of the window
	XTextProperty title;
	char* name = "Window title unavailable";
	if (XGetTextProperty(display, sxwmData->focusedWindow, &title, XInternAtom(display, "_NET_WM_NAME", False)))
		name = title.value;
	
	XDrawString(display, sxwmData->barWindow, gc, xpos + 10, 20, name, strlen(name));
}