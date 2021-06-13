#include "settings.h"
#include "util.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Default settings
BarSettings barSettings = {
	30,
	0x00222222,
	0x003333FF,
	0x00FFFFFF,
	0x00FFFFFF
};

Settings settings = {
	1,
	0x00FF0000
};

TileSettings tileSettings = {
	0.6f,
	1,
	10
};

KeyCombo* rootKeyCombos = NULL;
KeyCombo* clientKeyCombos = NULL;

void keybind(char* line, int start, int lineSize);

// Read a configuration from a given file
void readSettings(char* path) {
	// Attempt to open the file in read mode
	FILE *f = fopen(path, "r");

	// If the file could not be opened, die
	if (f == NULL) die("Could not open configuration file.");

	// A memory allocation we might later resize to fit long lines in
	int lineSize = 256;
	char* line = calloc(lineSize, 1);

	if (line == NULL) {
		fclose(f);
		die("Could not allocate memory.");
	}

	// Read the file until we reach the end
	while (fgets(line, lineSize, f) != NULL) {
		// We might not have read the entire line, if that is the case, then keep reading and
		// increasing the size of the line buffer.
		while (line[lineSize - 2] != 0 && line[lineSize - 2] != '\n') {
			// Expand the line buffer
			line = realloc(line, lineSize + 256);
			if (line == NULL) {
				fclose(f);
				die("Could not allocate memory.");
			}
			// Fill the new part of the buffer with zeros.
			memset(line + lineSize, 0, 256);
			// Read more of the line.
			fgets(line + lineSize - 1, 257, f);
			lineSize += 256;
		}
		// At this point, line contains a whole line from the file, and can be of length up to
		// lineSize - 1.

		// Read through the line until the first non-whitespace character is found
		int p = 0; // The index of the first non-whitespace character.

		while (line[p] == ' ' || line[p] == '\t') p++;

		// If the first character is a newline or a # (for a comment) then we can skip this line.
		if (line[p] == '\n' || line[p] == '#') continue;

		// Find how many characters are in the first word of this line
		int l = 0;
		while (line[p+l] != ' ' && line[p+l] != '\t' && line[p+l] != 0) l++;

		// Compare to find the appropriate command and call that function
		switch (l) {
			case 4:
				if (memcmp(line + p, "bind", 4) == 0) keybind(line, p, lineSize);
				else {
					fclose(f);
					free(line);
					die("Command not recognised.");
				}
				break;
			default: {
				fclose(f);
				free(line);
				die("Command not recgnised.");
			}
		}

		// Zero the memory of the line buffer again.
		memset(line, 0, lineSize);
	}

	// Clean up
	free(line);
	fclose(f);

	die("a");
}

// Save a keybinding to the relevant data structure
void keybind(char* line, int start, int lineSize) {
	printf("%s", line + start);
}