#include "../../clients.h"
#include "../../monitors.h"
#include "../../settings.h"
#include "../../sxwm.h"
#include "../../workspaces.h"

#include <stdio.h>

extern TileSettings tileSettings;
extern Settings settings;
extern BarSettings barSettings;
extern Display* display;
extern struct Monitor *selectedMonitor;

// Aranges the windows on the screen into the tiling layout
void tile()
{
	struct Workspace *workspace = selectedMonitor->workspaces;
	// Count the number of non-floating windows
	int toTile = 0;
	for (struct Client *c = workspace->clients; c != NULL; c=c->next)
		if (!(c->floating) && (c->tags & sxwmData->currentTags)) toTile++;
	
	// If there are no windows to tile, then do nothing
	//if (toTile == 0) return;
	if (toTile == 0) toTile = 1;
	
	// Determine the dimensions for master and slave windows
	int mw, mh, sw, sh;
	if (tileSettings.masterCount >= toTile) {
		mw = workspace->width - (2 * tileSettings.gapSize);
		mh = (int)((workspace->height - barSettings.height - (tileSettings.gapSize * (toTile + 1))) / toTile);
		sw = 0;
		sh = 0;
	} else {
		mw = (workspace->width * tileSettings.masterRatio) - (int)(1.5 * tileSettings.gapSize);
		mh = (int)((workspace->height - barSettings.height - (tileSettings.gapSize * (tileSettings.masterCount + 1))) / tileSettings.masterCount);
		sw = (workspace->width * (1 - tileSettings.masterRatio)) - (int)(1.5 * tileSettings.gapSize);
		sh = (int)((workspace->height - barSettings.height - (tileSettings.gapSize * (toTile - tileSettings.masterCount + 1))) / (toTile - tileSettings.masterCount));
	}

	// Organise the windows
	int i = 0;
	for (struct Client *c = workspace->clients; c != NULL; c=c->next) {
		// Put tiled windows in the correct place
		if (!(c->floating) && (c->tags & sxwmData->currentTags)) {
			XResizeWindow(display, c->frame, i < tileSettings.masterCount ? mw : sw, i < tileSettings.masterCount ? mh : sh);
			XResizeWindow(display, c->window, i < tileSettings.masterCount ? mw : sw, i < tileSettings.masterCount ? mh : sh);
			XMoveWindow(display, c->frame, (i < tileSettings.masterCount ? tileSettings.gapSize : mw + (2 * tileSettings.gapSize)) - settings.borderWidth,
			                                (barSettings.height + (i < tileSettings.masterCount ? (tileSettings.gapSize * (i+1)) + mh*i : (tileSettings.gapSize * (i-tileSettings.masterCount+1)) + sh*(i-tileSettings.masterCount)))) - settings.borderWidth;

			i++;
		} 
		// Put floating windows back where they should be
		else if (c->tags & sxwmData->currentTags) {
			XMoveWindow(display, c->frame, c->floatingx, c->floatingy);
		}
		// Move other windows off the screen
		else {
			XMoveWindow(display, c->frame, workspace->width * -2, 0);
		}
	}
}