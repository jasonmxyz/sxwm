#pragma once

typedef struct BarSettings BarSettings;
struct BarSettings {
	int height;
	long bgColor1;
	long bgColor2;
	long fgColor1;
	long fgColor2;
};

typedef struct Settings Settings;
struct Settings {
	int borderWidth;
	long borderColor;
};

typedef struct TileSettings TileSettings;
struct TileSettings {
	float masterRatio;
	int masterCount;
	int gapSize;
};