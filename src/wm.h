#pragma once

#include <X11/Xlib.h>

/* We maintain a linked list of these workspaces on each monitor. */
struct Workspace {
	int x;
	int y;
	int width;
	int height;
	struct Client *clients;
	struct Client *focused;
	int clientCount;
	struct WorkspaceDescription *wd;
	struct FrameDescription *fd;
	struct Workspace *prev;
	struct Workspace *next;
};

/* We maintain a linked list of these structures representing the avialable
 * types of workspace. */
struct WorkspaceDescription {
	int id;
	char *name;
	struct WorkspaceDescription *prev;
	struct WorkspaceDescription *next;
	void (*newClient)(struct Workspace*, Window);
	void (*removeClient)(struct Workspace*, struct Client*);
};

/* The client structure stores the window(s) associated with some client
 * window, and connects it to other clients in the same workspace by means of
 * two linked lists. */
struct Client {
	Window frame;
	Window window;
	struct Client *next;
	struct Client *previous;
	struct Client *focusNext;
	struct Client *focusPrevious;
	int floating;
	int tags;
	int floatingx;
	int floatingy;
};

/* We may provide instructions about how to place windows on the screen using
 * this structure. We can ask that the frame or client window be in a location
 * on the screen, or that the frame or client have some size. Bits in 'mask'
 * indicate the preferences. */
struct FrameSizePosHint {
	int frameX;
	int frameY;
	int clientX;
	int clientY;
	int frameW;
	int frameH;
	int clientW;
	int clientH;
	int mask;
};

/* Windows are reparented within frames, different types of frames can have
 * different behavior, we track a linked list of these structures which can be
 * selected by workspaces. */
struct FrameDescription {
	int id;
	char *name;
	struct FrmaeDescription *prev;
	struct FrmaeDescription *next;
	void (*create)(struct Workspace*, struct Client*, struct FrameSizePosHint*);
	void (*destroy)(struct Workspace*, struct Client*);
	void (*moveresize)(struct Workspace*, struct Client*, struct FrameSizePosHint*);
};

/* Bits for the FrameSizePosHint mask variable. */
#define FRAME_FX (1 << 0)
#define FRAME_FY (1 << 1)
#define FRAME_FW (1 << 2)
#define FRAME_FH (1 << 3)
#define FRAME_CX (1 << 4)
#define FRAME_CY (1 << 5)
#define FRAME_CW (1 << 6)
#define FRAME_CH (1 << 7)
/* More for convenience. */
#define FRAME_FS (FRAME_FW | FRAME_FH)
#define FRAME_FP (FRAME_FX | FRAME_FY)
#define FRAME_CS (FRAME_CW | FRAME_CH)
#define FRAME_CP (FRAME_CX | FRAME_CY)