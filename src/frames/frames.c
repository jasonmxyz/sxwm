#include "../frames.h"

extern void create(struct Workspace *workspace, struct Client *client, struct FrameSizePosHint *hint);
extern void destroy(struct Workspace *workspace, struct Client *client);

struct FrameDescription basic = {
	.id = 1,
	.name = "basic",
	.prev = NULL,
	.next = NULL,
	.create = create,
	.destroy = destroy
};

struct FrameDescription *frameDescriptions = &basic;
struct FrameDescription *defaultFrame = &basic;