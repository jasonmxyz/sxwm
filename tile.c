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
	// Count the number of non-floating windows
	int toTile = 0;
	for (Client* c = clients; c != NULL; c=c->next)
		if (!(c->floating)) toTile++;
	
	// If there are no windows to tile, then do nothing
	if (toTile == 0) return;
	
	// Determine the dimensions for master and slave windows
	int mw, mh, sw, sh;
	if (settings->masterCount >= toTile) {
		mw = monitor->width - (2 * settings->gapSize);
		mh = (int)((monitor->height - (settings->gapSize * (toTile + 1))) / toTile);
		sw = 0;
		sh = 0;
	} else {
		mw = (monitor->width * settings->masterRatio) - (int)(1.5 * settings->gapSize);
		mh = (int)((monitor->height - (settings->gapSize * (settings->masterCount + 1))) / settings->masterCount);
		sw = (monitor->width - (int)(1.5 * settings->gapSize) - mw);
		sh = (int)((monitor->height - (settings->gapSize * (toTile - settings->masterCount + 1))) / (toTile - settings->masterCount));
	}

	// Organise the windows
	int i = 0;
	for (Client* c = clients; c != NULL; c=c->next) {
		if (c->floating) continue;
		XResizeWindow(display, c->frame, i < settings->masterCount ? mw : sw, i < settings->masterCount ? mh : sh);
		XResizeWindow(display, c->window, i < settings->masterCount ? mw : sw, i < settings->masterCount ? mh : sh);
		XMoveWindow(display, c->frame, i < settings->masterCount ? settings->gapSize : mw + (2 * settings->gapSize),
		                               i < settings->masterCount ? (settings->gapSize * (i+1)) + mh*i : (settings->gapSize * (i-settings->masterCount)+1) + sh*(i-settings->masterCount));
		i++;
	}
}