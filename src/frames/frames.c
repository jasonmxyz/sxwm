#include "../frames.h"

extern void create(struct Workspace *workspace, struct Client *client, struct FrameSizePosHint *hint);

struct FrameDescription basic = {
	.id = 1,
	.name = "basic",
	.prev = NULL,
	.next = NULL,
	.create = create
};

struct FrameDescription *frameDescriptions = &basic;
struct FrameDescription *defaultFrame = &basic;