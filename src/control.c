#include "clients.h"
#include "monitors.h"
#include "sxwm.h"
#include "workspaces.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern void tile();
int running = 1;
extern Display* display;
extern struct Monitor *selectedMonitor;

// Kill the focused client
void killFocusedWindow()
{
	// Determine the currently focused client
	struct Workspace *workspace = selectedMonitor->workspaces;
	struct Client *focused = workspace->focused;

	if (!focused) {
		return;
	}

	// Kill the window in that client
	XGrabServer(display);
	XSetCloseDownMode(display, DestroyAll);
	XKillClient(display, focused->window);
	XSync(display, 0);
	XUngrabServer(display);
}

// Toggle the floating state of the focused window
void toggleFloating()
{
	// Determine the focused client
	struct Workspace *workspace = selectedMonitor->workspaces;
	struct Client *focused = workspace->focused;

	if (!focused) {
		return;
	}

	// Toggle the floating status of the window, and raise or lower the window
	if (focused->floating) XLowerWindow(display, focused->frame);
	else XRaiseWindow(display, focused->frame);
	focused->floating = focused->floating ? 0 : 1;

	// Tile the screen
	tile();
	
	return;
}

// Selects the specified tag if possible, and retile the windows if necessary.
void selectTag(int t)
{
	struct Workspace *workspace = selectedMonitor->workspaces;

	// If t is outside the range of possible tags, then do nothing.
	if (t < 0 || t > sizeof(int)*8) return;

	if (t == 0) t = -1;
	else t = 1 << (t-1);

	// If t is already the currently selected tag, then do nothing.
	if (t == sxwmData->currentTags) return;

	// Set the tag
	sxwmData->currentTags = t;

	// Find the focused window in the tag
	Window focused = 0;
	for (struct Client *front = workspace->focused; front != NULL; front = front->focusNext)
		if (front->tags & t) {
			focused = front->window;
			break;
		}
	sxwmData->focusedWindow = focused;

	// Send an expose message to the bar
	if (sxwmData->barWindow != 0) {
		XEvent event;
		event.type = Expose;
		XSendEvent(display, sxwmData->barWindow, 1, NoEventMask, &event);
	}

	// Retile the window
	tile();
}

// Stops sxwm
void stop()
{
	running = 0;
}

// Run a program with execvp
void startProgram(char* command, int newSession)
{
	int used = 0;
	int size = 10;
	char** cmd;
	int inQuotes = 0;
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
		if (newSession) setsid();
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

void runCmd(char* command) { startProgram(command, 1); }