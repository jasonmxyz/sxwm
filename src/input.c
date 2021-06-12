#include "clientList.h"
#include "util.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

extern Display* display;
extern Window root;
extern bool running;
extern Shared* shared;

extern int errorHandler(Display* display, XErrorEvent* e);
extern int nothingHandler(Display* display, XErrorEvent* e);

extern void tile();

Point mouseDownPos;
Point initialFramedPos;
Dimension initialFramedSize;

char* stArgv[] = {"st", NULL};
char* dmenuArgv[] = {"dmenu_run", NULL};

void keyPress(XEvent e) {
	// If the input is on the root window
	if (e.xkey.window == root) {
		if ((e.xkey.state & (Mod4Mask | ShiftMask)) && (e.xkey.keycode == XKeysymToKeycode(display, XK_q))) {
			running = false;
			return;
		}
		if ((e.xkey.state & (Mod4Mask | ShiftMask)) && (e.xkey.keycode == XKeysymToKeycode(display, XK_p))) {
			// Start dmenu
			if (fork() == 0) {
				setsid();
				execvp(dmenuArgv[0], dmenuArgv);
				exit(0);
			}
			return;
		}
		if ((e.xkey.state & (Mod4Mask | ShiftMask)) && (e.xkey.keycode == XKeysymToKeycode(display, XK_Return))) {
			// Start a new process by forking
			if (fork() == 0) {
				setsid();
				execvp(stArgv[0], stArgv);
				exit(0);
			}
			return;
		}
		if (e.xkey.state & Mod4Mask) {
			if (e.xkey.keycode == XKeysymToKeycode(display, XK_1)) {
					shared->currentTag = 1;
			} else if (e.xkey.keycode == XKeysymToKeycode(display, XK_2)) {
					shared->currentTag = 2;
			} else if (e.xkey.keycode == XKeysymToKeycode(display, XK_3)) {
					shared->currentTag = 3;
			} else if (e.xkey.keycode == XKeysymToKeycode(display, XK_4)) {
					shared->currentTag = 4;
			} else if (e.xkey.keycode == XKeysymToKeycode(display, XK_5)) {
					shared->currentTag = 5;
			} else if (e.xkey.keycode == XKeysymToKeycode(display, XK_6)) {
					shared->currentTag = 6;
			} else if (e.xkey.keycode == XKeysymToKeycode(display, XK_7)) {
					shared->currentTag = 7;
			} else if (e.xkey.keycode == XKeysymToKeycode(display, XK_8)) {
					shared->currentTag = 8;
			} else if (e.xkey.keycode == XKeysymToKeycode(display, XK_9)) {
					shared->currentTag = 9;
			}
			// Send an expose message to the bar window if it exists
			if (shared->bar != (Window)NULL) {
				XEvent event;
				event.type = Expose;
				XSendEvent(display, shared->bar, true, NoEventMask, &event);
			}
			tile();
		}
	}
	
	// Get client associated with this window. If there isn't one, then do nothing
	Client* c = getClientByWindow(e.xkey.window);
	if (c == NULL) return;

	// If the Mod+Space combination was given, we should toggle some window to be floating or not
	if ((e.xkey.state & Mod4Mask) && (e.xkey.keycode == XKeysymToKeycode(display, XK_space))) {
		// Toggle floating, call the tile method, and bring the window frame to the front
		if (c->floating) XLowerWindow(display, c->frame);
		else XRaiseWindow(display, c->frame);
		c->floating = c->floating ? false : true;
		tile();
		return;
	}

	// If the Mod+Shift+c combination was recieved, we should close some window
	if ((e.xkey.state & (Mod4Mask | ShiftMask)) && (e.xkey.keycode == XKeysymToKeycode(display, XK_c))) {
		XGrabServer(display);
		XSetErrorHandler(nothingHandler);
		XSetCloseDownMode(display, DestroyAll);
		XKillClient(display, c->window);
		XSync(display, false);
		XSetErrorHandler(errorHandler);
		XUngrabServer(display);
		return;
	}
}

void buttonPress(XEvent e) {
	// Get client associated with this window. If there isn't one, then do nothing
	Client* c = getClientByWindow(e.xbutton.window);
	if (c == NULL) return;
	
	// Get the geometry of the window
	Window r;// unused variables
	unsigned int bw, d;
	XGetGeometry(display, c->frame, &r, &(initialFramedPos.x), &(initialFramedPos.y), &(initialFramedSize.x), &(initialFramedSize.y), &bw, &d);

	// If right clicking, then move the mouse to the buttom right corner of the window.
	if (e.xbutton.button == Button3) {
		mouseDownPos.x = initialFramedPos.x + initialFramedSize.x;
		mouseDownPos.y = initialFramedPos.y + initialFramedSize.y;
	} else {
		// Record the position the mouse was clicked, and the position of the window
		mouseDownPos.x = e.xbutton.x_root;
		mouseDownPos.y = e.xbutton.y_root;
	}

	// Make sure the clicked window is on top
	XRaiseWindow(display, c->frame);
}

void motionNotify(XEvent e) {
	// Get client associated with this window. If there isn't one, then do nothing
	Client* c = getClientByWindow(e.xmotion.window);
	if (c == NULL) return;

	// If a window is being resized
	if (e.xmotion.state & Button3Mask) {
		// Resize both the window and its frame
		XResizeWindow(display, c->frame, initialFramedSize.x - (mouseDownPos.x - e.xmotion.x_root), initialFramedSize.y - (mouseDownPos.y - e.xmotion.y_root));
		XResizeWindow(display, c->window, initialFramedSize.x - (mouseDownPos.x - e.xmotion.x_root), initialFramedSize.y - (mouseDownPos.y - e.xmotion.y_root));
		if (!(c->floating)) {
			// Preserve its floating location
			(c->floatingLocation).x = initialFramedPos.x;
			(c->floatingLocation).y = initialFramedPos.y;
			c->floating = true;
			tile();
		}
	}

	// If a window was dragged
	if (e.xmotion.state & Button1Mask) {
		XMoveWindow(display, c->frame, initialFramedPos.x - (mouseDownPos.x - e.xmotion.x_root), initialFramedPos.y - (mouseDownPos.y - e.xmotion.y_root));
		(c->floatingLocation).x = initialFramedPos.x - (mouseDownPos.x - e.xmotion.x_root);
		(c->floatingLocation).y = initialFramedPos.y - (mouseDownPos.y - e.xmotion.y_root);
		// Bring the window out of floating mode, and retile the screen if necessary
		if (!(c->floating)) {
			c->floating = true;
			tile();
		}
	}
}