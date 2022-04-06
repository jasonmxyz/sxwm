#pragma once

#include "../../wm.h"

#include <X11/Xlib.h>

void newClient(struct Workspace *workspace, Window window);
void removeClient(struct Workspace *workspace, struct Client *client);