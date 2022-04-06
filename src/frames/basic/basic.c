#include "basicFrame.h"
#include "../../util.h"
#include "../../wm.h"

#include <X11/Xlib.h>

extern Display *display;
extern Window root;

/*
 * Creates a frame around a client window stored in client->window with the
 * size and position given in the hint.
 *
 * At least one X, Y, W, and H value must be specified in the hint mask.
 */
void create(struct Workspace *workspace, struct Client *client, struct FrameSizePosHint *hint)
{
	/* The client should not already have a frame. */
	if (client->frame) {
		die("Client is already framed.");
	}

	/* Check the mask is valid. */
	int mask = hint->mask;
	if (
		((mask & FRAME_FX) && (mask & FRAME_CX)) ||
		((mask & FRAME_FY) && (mask & FRAME_CY)) ||
		((mask & FRAME_FW) && (mask & FRAME_CH)) ||
		((mask & FRAME_FH) && (mask & FRAME_CH)))
	{
		die("Invalid mask.");
	}

	/* Determine the size and position of both the frame and client
	 * window. */
	int fx, fy, cx, cy;
	int fw, fh, cw, ch;
	if (mask & FRAME_FX) {
		fx = hint->frameX;
		cx = BF_LEFT;
	} else {
		fx = hint->clientX - BF_LEFT;
		cx = BF_LEFT;
	}
	if (mask & FRAME_FY) {
		fy = hint->frameY;
		cy = BF_TOP;
	} else {
		fy = hint->clientY - BF_TOP;
		cy = BF_TOP;
	}
	if (mask & FRAME_FW) {
		fw = hint->frameW;
		cw = fw - BF_LEFT - BF_RIGHT;
	} else {
		cw = hint->clientW;
		fw = cw + BF_LEFT + BF_RIGHT;
	}
	if (mask & FRAME_FH) {
		fh = hint->frameH;
		ch = fh - BF_TOP - BF_BOTTOM;
	} else {
		ch = hint->clientH;
		fh = hint->clientX + BF_TOP + BF_BOTTOM;
	}

	/* Create the frame window and reparent the client within it at the
	 * correct position. */
	client->frame = XCreateSimpleWindow(display, root, fx, fy, fw, fh, 0, 0, BF_BG);
	XSelectInput(display, client->frame, SubstructureRedirectMask | SubstructureNotifyMask | EnterWindowMask);

	/* Reparent the client and map the windows. */
	XReparentWindow(display, client->window, client->frame, cx, cy);
	XResizeWindow(display, client->window, cw, ch);
	XMapWindow(display, client->window);
	XMapWindow(display, client->frame);

	// Grab the required keys from the frame
	XGrabButton(display, Button1, Mod4Mask, client->window, 1, ButtonPressMask | ButtonReleaseMask | ButtonMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(display, Button3, Mod4Mask, client->window, 1, ButtonPressMask | ButtonReleaseMask | ButtonMotionMask, GrabModeAsync, GrabModeAsync, None, None);
}

/*
 * Move and resize the frame an client window according to the given hint.
 *
 * At least one X, Y, W, and H value must be specified in the hint mask.
 */
void moveresize(struct Workspace *workspace, struct Client *client, struct FrameSizePosHint *hint)
{
	/* The client should already have a frame. */
	if (!client->frame) {
		die("Attempt to configure a client with no frame.");
	}

	/* Check the mask is valid. */
	int mask = hint->mask;
	if (
		((mask & FRAME_FX) && (mask & FRAME_CX)) ||
		((mask & FRAME_FY) && (mask & FRAME_CY)) ||
		((mask & FRAME_FW) && (mask & FRAME_CH)) ||
		((mask & FRAME_FH) && (mask & FRAME_CH)))
	{
		die("Invalid mask.");
	}

	/* Determine the size and position of both the frame and client
	 * window. */
	int fx, fy, cx, cy;
	int fw, fh, cw, ch;
	if (mask & FRAME_FX) {
		fx = hint->frameX;
		cx = BF_LEFT;
	} else {
		fx = hint->clientX - BF_LEFT;
		cx = BF_LEFT;
	}
	if (mask & FRAME_FY) {
		fy = hint->frameY;
		cy = BF_TOP;
	} else {
		fy = hint->clientY - BF_TOP;
		cy = BF_TOP;
	}
	if (mask & FRAME_FW) {
		fw = hint->frameW;
		cw = fw - BF_LEFT - BF_RIGHT;
	} else {
		cw = hint->clientW;
		fw = cw + BF_LEFT + BF_RIGHT;
	}
	if (mask & FRAME_FH) {
		fh = hint->frameH;
		ch = fh - BF_TOP - BF_BOTTOM;
	} else {
		ch = hint->clientH;
		fh = hint->clientX + BF_TOP + BF_BOTTOM;
	}

	XMoveResizeWindow(display, client->frame, fx, fy, fw, fh);
	XMoveResizeWindow(display, client->window, cx, cy, cw, ch);
}

/*
 * Destroys the frame around the window, preforming any cleanup necessary.
 * Unmaps the client window and reparents it under the root window.
 */
void destroy(struct Workspace *workspace, struct Client *client)
{
	XUnmapWindow(display, client->window);
	XReparentWindow(display, client->window, root, 0, 0);
	
	if (client->frame) {
		XUnmapWindow(display, client->frame);
		XDestroyWindow(display, client->frame);
	}

	client->frame = 0;
}