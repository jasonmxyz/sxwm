#include "clientList.h"
#include "tile.h"
#include "monitors.h"

#include <stdio.h>

extern tileSettings* settings;
extern Monitor* monitor;
extern Client* clients;
extern int clientCount;
extern Display* display;
extern int barHeight;
extern int borderWidth;
extern int currentTag;

// Aranges the windows on the screen into the tiling layout
void tile() {
	// Count the number of non-floating windows
	int toTile = 0;
	for (Client* c = clients; c != NULL; c=c->next)
		if (!(c->floating) && (c->tag == currentTag)) toTile++;
	
	// If there are no windows to tile, then do nothing
	//if (toTile == 0) return;
	if (toTile == 0) toTile = 1;
	
	// Determine the dimensions for master and slave windows
	int mw, mh, sw, sh;
	if (settings->masterCount >= toTile) {
		mw = monitor->width - (2 * settings->gapSize);
		mh = (int)((monitor->height - barHeight - (settings->gapSize * (toTile + 1))) / toTile);
		sw = 0;
		sh = 0;
	} else {
		mw = (monitor->width * settings->masterRatio) - (int)(1.5 * settings->gapSize);
		mh = (int)((monitor->height - barHeight - (settings->gapSize * (settings->masterCount + 1))) / settings->masterCount);
		sw = (monitor->width * (1 - settings->masterRatio)) - (int)(1.5 * settings->gapSize);
		sh = (int)((monitor->height - barHeight - (settings->gapSize * (toTile - settings->masterCount + 1))) / (toTile - settings->masterCount));
	}

	// Organise the windows
	int i = 0;
	for (Client* c = clients; c != NULL; c=c->next) {
		// Put tiled windows in the correct place
		if (!(c->floating) && (c->tag == currentTag)) {
			XResizeWindow(display, c->frame, i < settings->masterCount ? mw : sw, i < settings->masterCount ? mh : sh);
			XResizeWindow(display, c->window, i < settings->masterCount ? mw : sw, i < settings->masterCount ? mh : sh);
			XMoveWindow(display, c->frame, (i < settings->masterCount ? settings->gapSize : mw + (2 * settings->gapSize)) - borderWidth,
			                                (barHeight + (i < settings->masterCount ? (settings->gapSize * (i+1)) + mh*i : (settings->gapSize * (i-settings->masterCount+1)) + sh*(i-settings->masterCount)))) - borderWidth;

			i++;
		} 
		// Put floating windows back where they should be
		else if (c->tag == currentTag) {
			XMoveWindow(display, c->frame, (c->floatingLocation).x, (c->floatingLocation).y);
		}
		// Move other windows off the screen
		else {
			XMoveWindow(display, c->frame, monitor->width * -2, 0);
		}
	}
}