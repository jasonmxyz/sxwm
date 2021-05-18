#pragma once

typedef struct tileSettings tileSettings;
struct tileSettings {
	float masterRatio;
	int masterCount;
	int gapSize;
};

void tile();