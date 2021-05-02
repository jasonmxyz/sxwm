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
		if (c->floating) toTile++;
	
	// Determine the dimensions for master and slave windows
	int mw = toTile > settings->masterCount ? (int)(monitor->width * settings->masterRatio) : monitor->width;
	int mh = monitor->height / (toTile < settings->masterCount ? toTile : settings->masterCount);
	int sw = monitor->width - mw;
	int sh = monitor->height / (toTile > settings->masterCount ? toTile - settings->masterCount : 1);

	// Organise the windows
	int i = 0;
	for (Client* c = clients; c != NULL; c=c->next) {
		XResizeWindow(display, c->frame, i < settings->masterCount ? mw : sw, i < settings->masterCount ? mh : sh);
		XResizeWindow(display, c->window, i < settings->masterCount ? mw : sw, i < settings->masterCount ? mh : sh);
		XMoveWindow(display, c->frame, i < settings->masterCount ? 0 : mw, i < settings->masterCount ? mh*i : sh*(i-settings->masterCount));
		i++;
	}
}