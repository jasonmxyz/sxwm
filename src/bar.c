#include "util.h"
#include "sxwm.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <stdio.h>

char** g_argv;

GC gc;
Display* display;
int width;

void draw();
int drawTags();
void drawTitle(int xpos);

int main(int argc, char** argv) {
	DEBUG("Start of bar process.");
	g_argv = argv;

	// Create the shared memory
	createShared(1);
	mapShared(1);
	DEBUG("Connected to shared memory.");

	display = XOpenDisplay(NULL);
	Window root = XDefaultRootWindow(display);
	
	width = DisplayWidth(display, DefaultScreen(display));
	
	XClassHint wc = {"sxwm", "sxwm-bar"};
	sxwmData->barWindow = XCreateSimpleWindow(display,
											  root,
											  0,
											  0,
											  width,
											  sxwmData->barSettings.height,
											  0,
											  sxwmData->barSettings.bgColor1,
											  sxwmData->barSettings.bgColor1);
	DEBUG("Created window.");
	XSelectInput(display, sxwmData->barWindow, ExposureMask);
	XSetClassHint(display, sxwmData->barWindow, &wc);
	XMapWindow(display, sxwmData->barWindow);
	DEBUG("Mapped window.");

	gc = XCreateGC(display, root, 0, NULL);

	XEvent e;
	while(1) {
		XNextEvent(display, &e);
		if (e.type == Expose) {
			draw();
		}
	}


	DEBUG("Closing Display.");
	XCloseDisplay(display);

	shm_unlink(shmName);
	DEBUG("Unlinked from shared memory.");
}

void draw() {
	int xpos = drawTags();
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

void drawTitle(int xpos) {	
	XSetForeground(display, gc, sxwmData->barSettings.fgColor1);

	// If there is no focused window, then draw some placeholder text
	if (!sxwmData->focusedWindow) {
		const char* msg = "github.com/jasonmxyz/sxwm";
		XDrawString(display, sxwmData->barWindow, gc, xpos + 10, 20, msg, strlen(msg));
		return;
	}

	// Get the title of the window
	XTextProperty title;
	XGetTextProperty(display, sxwmData->focusedWindow, &title, XInternAtom(display, "_NET_WM_NAME", False));
	XDrawString(display, sxwmData->barWindow, gc, xpos + 10, 20, title.value, strlen(title.value));

}