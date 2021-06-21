#pragma once

#include <X11/Xlib.h>

// Settings that the window manager wants the bar to use.
typedef struct BarSettings BarSettings;
struct BarSettings {
	int height;
	long bgColor1;
	long bgColor2;
	long fgColor1;
	long fgColor2;
};

typedef struct SXWMData SXWMData;
struct SXWMData {
	int currentTags;
	int windowCounts[sizeof(int)*8];
	Window focusedWindow;
	Window barWindow;
	BarSettings barSettings;
};

int createShared(int write);
void mapShared(int write);

extern SXWMData* sxwmData;
extern char* shmName;