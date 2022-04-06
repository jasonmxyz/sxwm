#pragma once

#include "wm.h"

#include <X11/Xlib.h>

int getClientWorkspace(Window window, struct Client **retClient, struct Workspace **retWorkspace);
int getClientWorkspaceAny(Window window, struct Client **retClient, struct Workspace **retWorkspace);