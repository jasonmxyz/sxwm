#include "clientList.h"
#include "tile.h"

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

extern Display* display;

void keyPress(XEvent e) {
	#ifdef VERBOSE
	printf("Recieved Key Press Event\n");
	#endif
	
	// Get client associated with this window. If there isn't one, then do nothing
	Client* c = getClientByWindow(e.xkey.window);
	if (c == NULL) return;

	// If the Mod+Space combination was given, we should toggle some window to be floating or not
	if ((e.xkey.state & Mod4Mask) && (e.xkey.keycode == XKeysymToKeycode(display, XK_space))) {
		#ifdef VERBOSE
		printf(" Toggle window floating mode\n");
		#endif
		// Toggle floating, call the tile method, and bring the window frame to the front
		c->floating = c->floating ? false : true;
		tile();
		XRaiseWindow(display, c->frame);
		return;
	}
}