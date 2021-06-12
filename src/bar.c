#include "util.h"

#include <stdlib.h>
#include <X11/Xlib.h>
#include <unistd.h>
#include <string.h>
#include <X11/Xutil.h>

extern int barHeight;

Window bar;
Display* d;
int s, width;
GC gc;

void draw();

void createBar() {
	int result = fork();
	if (result < 0) die("Could not create another thread");
	if (result != 0) return;

	d = XOpenDisplay(NULL);
	Window r = XDefaultRootWindow(d);
	s = DefaultScreen(d);
	width = DisplayWidth(d, s);

	XClassHint wc = {"sxwm", "sxwm-bar"};
	bar = XCreateSimpleWindow(d, RootWindow(d, s), 0, 0, width, barHeight, 0, BlackPixel(d, s), WhitePixel(d, s));
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

	XSetForeground(d, gc, 0x000000FF);
	XFillRectangle(d, bar, gc, 0, 0, barHeight * 9, barHeight);

	XSetForeground(d, gc, 0x00000000);
	XDrawString(d, bar, gc, barHeight * 9 + 10, 20, msg, strlen(msg));
}