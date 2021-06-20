#include "util.h"
#include "memory.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

extern void tile();

// Selects the specified tag if possible, and retile the windows if necessary.
void selectTag(int t) {
	// If t is outside the range of possible tags, then do nothing.
	if (t < 0 || t > sizeof(int)*8) return;

	if (t == 0) t = -1;
	else t = 1 << (t-1);

	// If t is already the currently selected tag, then do nothing.
	if (t == shared->currentTags) return;

	// Set the tag
	shared->currentTags = t;

	// Retile the window
	tile();
}

// Stops sxwm
void stop() {
	shared->running = false;
}

// Run a command in a new thread
void runCmd(char* command) {
	int used = 0;
	int size = 10;
	char** cmd;
	bool inQuotes = false;
	int argStart = 0;

	cmd = calloc(10, sizeof(char*));

	// Look at every character in the string i.e. until we see 0
	for (int p = 0;; p++) {
		// If we are on a quote, then toggle inQuotes
		if (command[p] == '\"') {
			inQuotes = !inQuotes;
		}
		
		// If we are not on whitespace, but we previously were, and we aren't in quotes, then this
		// is a new argument
		if (p > 0 && command[p] != ' ' && command[p-1] == ' ' && !inQuotes) {
			argStart = p;
		}

		// If we are on a whitespace character, and the previous character was not a whitespace
		// character, and we are not in quotes, then we just finished an argument.
		if ((command[p] == ' ' || command[p] == 0) && command[p-1] != ' ' && !inQuotes) {
			// If we were previously in quotes, then we should not copy " into the string
			if (command[p-1] == '\"') {
				argStart++;
				p--;
			}
			// Copy this argument into a string
			cmd[used] = malloc(p+1-argStart);
			memcpy(cmd[used], command + argStart, p - argStart);
			cmd[used++][p-argStart] = 0;
			// If we have used all the arguments, expand the array.
			if (used == size) {
				char** new = calloc(size + 10, 1);
				memcpy(new, cmd, size);
				size += 10;
				char** old = cmd;
				cmd = new;
				free(old);
			}
			// Fix quote issue thing
			if (command[p] == '\"') p++;
		}
		// If we reached the end of the string, then exit
		if (command[p] == 0) break;
	}

	cmd[used] = NULL;

	// Now we can fork and execute the program
	if (fork() == 0) {
		setsid();
		execvp(cmd[0], cmd);
		// Free memory
		for (int i = 0; i < used-1; i++) if (cmd[i] != NULL) free(cmd[i]);
		free(cmd);
		exit(0);
	}

	// Free memory
	for (int i = 0; i < used-1; i++) if (cmd[i] != NULL) free(cmd[i]);
	free(cmd);
}