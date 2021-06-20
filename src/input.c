#include "clients.h"
#include "util.h"
#include "settings.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

extern Display* display;
extern Window root;
extern bool running;
extern KeyCombo* rootKeyCombos;
extern KeyCombo* clientKeyCombos;

extern int errorHandler(Display* display, XErrorEvent* e);
extern int nothingHandler(Display* display, XErrorEvent* e);

extern void tile();

extern void selectTag(int t);
extern void stop();

Point mouseDownPos;
Point initialFramedPos;
Dimension initialFramedSize;

void keyPress(XEvent e) {
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