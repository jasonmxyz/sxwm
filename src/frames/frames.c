#include "../wm.h"

extern void create(struct Workspace *workspace, struct Client *client, struct FrameSizePosHint *hint);
extern void moveresize(struct Workspace *workspace, struct Client *client, struct FrameSizePosHint *hint);
extern void destroy(struct Workspace *workspace, struct Client *client);

struct FrameDescription basic = {
	.id = 1,
	.name = "basic",
	.prev = NULL,
	.next = NULL,
	.create = create,
	.destroy = destroy,
	.moveresize = moveresize
};

struct FrameDescription *frameDescriptions = &basic;
struct FrameDescription *defaultFrame = &basic;