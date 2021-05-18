#pragma once

// The typename Screen is not available
typedef struct Monitor Monitor;
struct Monitor {
	int width, height;
};

void getMonitors();