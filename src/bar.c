#include "util.h"

#include <stdlib.h>
#include <X11/Xlib.h>
#include <unistd.h>
#include <string.h>
#include <X11/Xutil.h>

Window bar = (Window)NULL;
extern int barHeight;

void createBar() {
	int result = fork();
	if (result < 0) die("Could not create another thread");
	if (result != 0) return;

	Display* d = XOpenDisplay(NULL);
	Window r = XDefaultRootWindow(d);
	int s = DefaultScreen(d);
	int width = DisplayWidth(d, s);
	XClassHint wc = {"sxwm", "sxwm-bar"};
	Window w = XCreateSimpleWindow(d, RootWindow(d,s), 0, 0, width, barHeight, 0, BlackPixel(d,s), WhitePixel(d,s));
	XSelectInput(d, w, ExposureMask | KeyPressMask);
	XSetClassHint(d, w, &wc);
	XMapWindow(d,w);

	XEvent e;
	const char *msg = "github.com/jasonmxyx/sxwm";
	for (;;) {
		XNextEvent(d, &e);
		if (e.type == Expose) {
			//XFillRectangle(d, w, DefaultGC(d, s), 20, 20, 10, 10);
			XDrawString(d, w, DefaultGC(d, s), 10, 20, msg, strlen(msg));
		}
	}

	XCloseDisplay(d);

	exit(0);
}