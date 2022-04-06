#include "tiling/tiling.h"
#include "../wm.h"

struct WorkspaceDescription tiling = {
	.id = 1,
	.name = "tiling",
	.prev = NULL,
	.next = NULL,
	.newClient = newClient,
	.removeClient = removeClient
};

struct WorkspaceDescription *workspaceDescriptions = &tiling;
struct WorkspaceDescription *defaultWorkspace = &tiling;