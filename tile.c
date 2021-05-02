#include "clientList.h"
#include "tile.h"
#include "monitors.h"

#include <stdio.h>

extern tileSettings* settings;
extern Monitor* monitor;
extern Client* clients;
extern int clientCount;
extern Display* display;

// Aranges the windows on the screen into the tiling layout
void tile() {
	#ifdef VERBOSE
	printf("Tiling windows.\n");
	#endif
	// If all the windows are in the master stack
	if (clientCount <= settings->masterCount) {
		#ifdef VERBOSE
		printf(" All in one stack.\n");
		#endif
		// Determine the dimensions of each window
		int w = monitor->width;
		int h = monitor->height / clientCount;

		// Place each window in the correct position
		int i = 0;
		for (Client* c = clients; c != NULL; c = c->next) {
			#ifdef VERBOSE
			printf("%d  ", i);
			#endif
			XResizeWindow(display, c->frame, w, h);
			XResizeWindow(display, c->window, w, h);
			XMoveWindow(display, c->frame, 0, h*i);
			i++;
		}
		#ifdef VERBOSE
		printf("\n");
		#endif
	}
	// Othewise, do the fancy layout
	else {
		#ifdef VERBOSE
		printf(" Two stacks.\n");
		#endif
		// Calculate the two sets of dimensions
		int mw = (int)(monitor->width * settings->masterRatio);
		int sw = monitor->width - mw;
		int mh = monitor->height / settings->masterCount;
		int sh = monitor->height / (clientCount-settings->masterCount);

		// Place each window
		int i = 0;
		for (Client* c = clients; c != NULL; c = c->next) {
			#ifdef VERBOSE
			printf("%d  ", i);
			#endif
			if (i < settings->masterCount) {
				XResizeWindow(display, c->frame, mw, mh);
				XResizeWindow(display, c->window, mw, mh);
				XMoveWindow(display, c->frame, 0, mh*i);
			} else {
				XResizeWindow(display, c->frame, sw, sh);
				XResizeWindow(display, c->window, sw, sh);
				XMoveWindow(display, c->frame, mw, sh*(i-settings->masterCount));
			}
			i++;
		}
		#ifdef VERBOSE
		printf("\n");
		#endif
	}
}