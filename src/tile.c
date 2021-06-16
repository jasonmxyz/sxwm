#include "clients.h"
#include "settings.h"
#include "shared.h"

#include <stdio.h>

extern TileSettings tileSettings;
extern Settings settings;
extern BarSettings barSettings;
extern Monitor* monitor;
extern Display* display;

// Aranges the windows on the screen into the tiling layout
void tile() {
	// Count the number of non-floating windows
	int toTile = 0;
	for (Client* c = monitor->clients; c != NULL; c=c->next)
		if (!(c->floating) && (c->tag == shared->currentTag)) toTile++;
	
	// If there are no windows to tile, then do nothing
	//if (toTile == 0) return;
	if (toTile == 0) toTile = 1;
	
	// Determine the dimensions for master and slave windows
	int mw, mh, sw, sh;
	if (tileSettings.masterCount >= toTile) {
		mw = monitor->width - (2 * tileSettings.gapSize);
		mh = (int)((monitor->height - barSettings.height - (tileSettings.gapSize * (toTile + 1))) / toTile);
		sw = 0;
		sh = 0;
	} else {
		mw = (monitor->width * tileSettings.masterRatio) - (int)(1.5 * tileSettings.gapSize);
		mh = (int)((monitor->height - barSettings.height - (tileSettings.gapSize * (tileSettings.masterCount + 1))) / tileSettings.masterCount);
		sw = (monitor->width * (1 - tileSettings.masterRatio)) - (int)(1.5 * tileSettings.gapSize);
		sh = (int)((monitor->height - barSettings.height - (tileSettings.gapSize * (toTile - tileSettings.masterCount + 1))) / (toTile - tileSettings.masterCount));
	}

	// Organise the windows
	int i = 0;
	for (Client* c = monitor->clients; c != NULL; c=c->next) {
		// Put tiled windows in the correct place
		if (!(c->floating) && (c->tag == shared->currentTag)) {
			XResizeWindow(display, c->frame, i < tileSettings.masterCount ? mw : sw, i < tileSettings.masterCount ? mh : sh);
			XResizeWindow(display, c->window, i < tileSettings.masterCount ? mw : sw, i < tileSettings.masterCount ? mh : sh);
			XMoveWindow(display, c->frame, (i < tileSettings.masterCount ? tileSettings.gapSize : mw + (2 * tileSettings.gapSize)) - settings.borderWidth,
			                                (barSettings.height + (i < tileSettings.masterCount ? (tileSettings.gapSize * (i+1)) + mh*i : (tileSettings.gapSize * (i-tileSettings.masterCount+1)) + sh*(i-tileSettings.masterCount)))) - settings.borderWidth;

			i++;
		} 
		// Put floating windows back where they should be
		else if (c->tag == shared->currentTag) {
			XMoveWindow(display, c->frame, (c->floatingLocation).x, (c->floatingLocation).y);
		}
		// Move other windows off the screen
		else {
			XMoveWindow(display, c->frame, monitor->width * -2, 0);
		}
	}
}