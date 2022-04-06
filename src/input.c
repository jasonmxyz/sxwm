#include "clients.h"
#include "settings.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>

extern Display* display;
extern Window root;
extern KeyCombo* rootKeyCombos;

extern int errorHandler(Display* display, XErrorEvent* e);
extern int nothingHandler(Display* display, XErrorEvent* e);

extern void tile();

typedef struct Point {
	int x;
	int y;
} Point;
typedef Point Dimension;

Point mouseDownPos;
Point initialFramedPos;
Dimension initialFramedSize;

void keyPress(XEvent e)
{
	// If the input is on the root window
	if (e.xkey.window == root) {
		// Search through the key combination list for the root window
		for (KeyCombo* front = rootKeyCombos; front != NULL; front = front->next)
			if ((front->modifiers & e.xkey.state) && e.xkey.keycode == front->keycode) {
				if (front->hasArg == 0) {
					void (*f)(void) = front->function;
					f();
				}
				else {
					void (*f)(void*) = front->function;
					f(front->arg);
				}
				return;
			}
	}
}

void buttonPress(XEvent e)
{
	// Get client associated with this window. If there isn't one, then do nothing
	struct Client *c;
	if (getClientWorkspace(e.xbutton.window, &c, NULL) < 0) return;
	
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

void motionNotify(XEvent e)
{
	// Get client associated with this window. If there isn't one, then do nothing
	struct Client *c;
	if (getClientWorkspace(e.xbutton.window, &c, NULL) < 0) return;

	// If a window is being resized
	if (e.xmotion.state & Button3Mask) {
		// Resize both the window and its frame
		XResizeWindow(display, c->frame, initialFramedSize.x - (mouseDownPos.x - e.xmotion.x_root), initialFramedSize.y - (mouseDownPos.y - e.xmotion.y_root));
		XResizeWindow(display, c->window, initialFramedSize.x - (mouseDownPos.x - e.xmotion.x_root), initialFramedSize.y - (mouseDownPos.y - e.xmotion.y_root));
		if (!(c->floating)) {
			// Preserve its floating location
			c->floatingx = initialFramedPos.x;
			c->floatingy = initialFramedPos.y;
			c->floating = 1;
			tile();
		}
	}

	// If a window was dragged
	if (e.xmotion.state & Button1Mask) {
		XMoveWindow(display, c->frame, initialFramedPos.x - (mouseDownPos.x - e.xmotion.x_root), initialFramedPos.y - (mouseDownPos.y - e.xmotion.y_root));
		c->floatingx = initialFramedPos.x - (mouseDownPos.x - e.xmotion.x_root);
		c->floatingy = initialFramedPos.y - (mouseDownPos.y - e.xmotion.y_root);
		// Bring the window out of floating mode, and retile the screen if necessary
		if (!(c->floating)) {
			c->floating = 1;
			tile();
		}
	}
}