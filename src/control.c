#include "util.h"

#include <stdbool.h>
#include <X11/Xlib.h>

extern Shared* shared;

extern Display* display;

extern void tile();

// Selects the specified tag if possible, and retile the windows if necessary.
void selectTag(int t) {
	// If t is outside the range of possible tags, then do nothing.
	if (t <= 0 || t > 9) return;

	// If t is already the currently selected tag, then do nothing.
	if (t == shared->currentTag) return;

	// Set the tag
	shared->currentTag = t;

	// Send an expose message to the bar if it exists
	if (shared -> bar != (Window)NULL) {
		XEvent event;
		event.type = Expose;
		XSendEvent(display, shared->bar, true, NoEventMask, &event);
	}

	// Retile the window
	tile();
}

// Stops sxwm
void stop() {
	shared->running = false;
}