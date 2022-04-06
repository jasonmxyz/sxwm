#pragma once

#include "clients.h"
#include "workspaces.h"

#include <X11/Xlib.h>

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
