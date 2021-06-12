#include "util.h"
#include "settings.h"

#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <unistd.h>
#include <string.h>
#include <X11/Xutil.h>

extern BarSettings barSettings;

Window bar;
Display* d;
int s, width;
GC gc;
extern currentTag;

void draw();
void drawTags();

void createBar() {
	int result = fork();
	if (result < 0) die("Could not create another thread");
	if (result != 0) return;

	d = XOpenDisplay(NULL);
	Window r = XDefaultRootWindow(d);
	s = DefaultScreen(d);
	width = DisplayWidth(d, s);

	XClassHint wc = {"sxwm", "sxwm-bar"};
	bar = XCreateSimpleWindow(d, RootWindow(d, s), 0, 0, width, barSettings.height, 0, barSettings.bgColor1, barSettings.bgColor1);
	XSelectInput(d, bar, ExposureMask | KeyPressMask);
	XSetClassHint(d, bar, &wc);
	XMapWindow(d, bar);

	gc = XCreateGC(d, r, 0, NULL);

	XEvent e;
	for (;;) {
		XNextEvent(d, &e);
		if (e.type == Expose) {
			printf("Drawing bar\n");
			draw();
		}
	}

	XCloseDisplay(d);

	exit(0);
}

void draw() {
	const char *msg = "github.com/jasonmxyx/sxwm";

	XSetForeground(d, gc, barSettings.fgColor1);
	XDrawString(d, bar, gc, barSettings.height * 9 + 10, 20, msg, strlen(msg));

	drawTags();
}

void drawTags() {
	// Draw the main background rectangle
	XSetForeground(d, gc, barSettings.bgColor2);
	XFillRectangle(d, bar, gc, 0, 0, barSettings.height * 9, barSettings.height);
	XSetForeground(d, gc, barSettings.fgColor2);

	char tag[] = {'1', 0};

	// Draw each of the tag numbers, and indicate the currently selected tag with another color.
	for (int i = 1; i <= 9; i++, tag[0]++) {
		// If this tag is slected, then draw another coloured rectangle
		if (i == currentTag) {
			XSetForeground(d, gc, barSettings.bgColor1);
			XFillRectangle(d, bar, gc, barSettings.height * (i-1), 0, barSettings.height, barSettings.height);
			XSetForeground(d, gc, barSettings.fgColor1);
			XDrawString(d, bar, gc, barSettings.height * (i-1) + 10, 20, tag, 1);
			XSetForeground(d, gc, barSettings.fgColor2);
		}
		// Otherwise, just draw the number
		XDrawString(d, bar, gc, barSettings.height * (i-1) + 10, 20, tag, 1);
	}
}