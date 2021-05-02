#include "clientList.h"
#include "tile.h"
#include "util.h"

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/keysym.h>

extern Display* display;
extern Window root;
extern bool running;

Point mouseDownPos;
Point initialFramedPos;
Dimension initialFramedSize;

void keyPress(XEvent e) {
	// If the input is on the root window
	if (e.xkey.window == root) {
		if ((e.xkey.state & (Mod4Mask | ShiftMask)) && (e.xkey.keycode == XKeysymToKeycode(display, XK_q))) {
			running = false;
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
}

void buttonPress(XEvent e) {
	// Get client associated with this window. If there isn't one, then do nothing
	Client* c = getClientByWindow(e.xbutton.window);
	if (c == NULL) return;

	// Record the position the mouse was clicked, and the position of the window
	mouseDownPos.x = e.xbutton.x_root;
	mouseDownPos.y = e.xbutton.y_root;
	Window r;// unused variables
	unsigned int bw, d;
	XGetGeometry(display, c->frame, &r, &(initialFramedPos.x), &(initialFramedPos.y), &(initialFramedSize.x), &(initialFramedSize.y), &bw, &d);

	// Make sure the clicked window is on top
	XRaiseWindow(display, c->frame);
}

void motionNotify(XEvent e) {
	// Get client associated with this window. If there isn't one, then do nothing
	Client* c = getClientByWindow(e.xmotion.window);
	if (c == NULL) return;

	// If a window was dragged
	if (e.xmotion.state & Button1Mask) {
		XMoveWindow(display, c->frame, initialFramedPos.x - (mouseDownPos.x - e.xmotion.x_root), initialFramedPos.y - (mouseDownPos.y - e.xmotion.y_root));
		// Bring the window out of floating mode, and retile the screen if necessary
		if (!(c->floating)) {
			c->floating = true;
			tile();
		}
	}
}