#include "workspaces.h"

struct WorkspaceDescription tiling = {
	.id = 1,
	.name = "tiling",
	.prev = NULL,
	.next = NULL
};

struct WorkspaceDescription *workspaceDescriptions = &tiling;
struct WorkspaceDescription *defaultWorkspace = &tiling;