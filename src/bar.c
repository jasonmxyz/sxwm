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

	XSetForeground(d, gc, barSettings.bgColor2);
	XFillRectangle(d, bar, gc, 0, 0, barSettings.height * 9, barSettings.height);

	XSetForeground(d, gc, barSettings.fgColor1);
	XDrawString(d, bar, gc, barSettings.height * 9 + 10, 20, msg, strlen(msg));
}