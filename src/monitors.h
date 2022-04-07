#pragma once

/* We maintain a linked list of these monitor structures to represent each
 * physical display attached to the display server. */
struct Monitor {
	char *name; /* The name of the display as given by xrandr. */
	int x;
	int y;
	int width;
	int height;
	int id;
	struct Monitor *prev;
	struct Monitor *next;
	struct Workspace *workspaces;
};