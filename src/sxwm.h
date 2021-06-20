#pragma once

typedef struct SXWMData SXWMData;
struct SXWMData {
	int currentTags;
	int windowCounts[sizeof(int)*8];
};

int createShared(int write);
void mapShared(int write);

extern SXWMData* sxwmData;
extern char* shmName;