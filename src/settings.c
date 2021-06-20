#include "settings.h"
#include "util.h"
#include "sxwm.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

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

CmdQueue* commandQueue = NULL;

// Arrays of bindable functions
extern void selectTag(int t);
extern void stop();
extern void runCmd(char* command);
fDict rootFunctions[3] = {{"selectTag", selectTag, FDICT_NEEDINT},
						  {"exit", stop, 0},
						  {"run", runCmd, FDICT_NEEDSTRING}};
fDict clientFunctions[0] = {};

#define RF_COUNT 3 // How many elements there are in the above arrays
#define CF_COUNT 0

void keybind(char* line, int start, int lineSize);
void doStmt(char* line, int start, int newSession);

extern Display* display;

// Read a configuration from a given file
void readSettings(char* path) {
	// If the path is not set, then check in some of the predefined locations for it.
	// TODO
	if (path == NULL) die("No configuration file specified.");

	// Attempt to open the file in read mode
	FILE *f = fopen(path, "r");

	// If the file could not be opened, die
	if (f == NULL) die("Could not open configuration file.");

	// A memory allocation we might later resize to fit long lines in
	int lineSize = 256;
	char* line = calloc(lineSize, 1);

	if (line == NULL) {
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
		while (line[p+l] != ' ' && line[p+l] != '\t' && line[p+l] != '\n' && line[p+l] != 0) l++;

		// Compare to find the appropriate command and call that function
		switch (l) {
			case 3:
				if (memcmp(line + p, "run", 3) == 0) doStmt(line, p, 0);
				else die("Command not recognised.");
				break;
			case 4:
				if (memcmp(line + p, "bind", 4) == 0) keybind(line, p, lineSize);
				else die("Command not recognised.");
				break;
			case 5:
				if (memcmp(line + p, "start", 5) == 0) doStmt(line, p, 1);
				else die("Command not recognised.");
				break;
			default: {
				die("Command not recgnised.");
			}
		}

		// Zero the memory of the line buffer again.
		memset(line, 0, lineSize);
	}

	// Clean up
	free(line);
	fclose(f);
}

// Run a command at an appropriate time
void doStmt(char* line, int start, int newSession) {
	// Move along to find the start of the command at the first non-whitespace character
	int p = start; // The start of the word 'run'
	while (line[p] != 0 && line[p] != ' ' && line[p] != '\n' && line[p] != '\t') p++;
	while (line[p] == ' ' || line[p] == '\t') p++;

	// If we reached the end of the line, then this line is invalid
	if (line[p] == 0 || line[p] == '\n') die("No command specified.\n> %s", line+start);

	// Find the end of the command
	int l = 1;
	while (line[p+l] != 0 && line[p+l] != '\n' && line[p+l] != '\t') l++;

	// Create a CmdQueue object
	CmdQueue* new = malloc(sizeof(CmdQueue));
	// Copy the command into a new string
	char* cmd = malloc(l + 1);
	memcpy(cmd, line + p, l);
	cmd[l] = 0;

	new->cmd = cmd;
	new->newSession = newSession;
	new->next = NULL;

	// Add this commands to the queue of commands to run later
	if (commandQueue == NULL) {
		commandQueue = new;
		return;
	}
	// Go to the end of the linked list
	CmdQueue* front = commandQueue;
	while (front->next != NULL) front = front->next;
	front->next = new;
}

// Save a keybinding to the relevant data structure
void keybind(char* line, int start, int lineSize) {
	// Find the position in the string where the key combination is written
	// Find the first whitespace after the first word, then find the first non-whitespace
	// If we ever find a newline or null byte, then we die.
	int pk = start;
	while (line[pk] != ' ' && line[pk] != '\t' && line[pk] != '\n' && line[pk] != 0) pk++;
	if (line[pk] == 0 || line[pk] == '\n') die("Too few arguments.\n> %s", line+start);
	while (line[pk] == ' ' || line[pk] == '\t') pk++;
	if (line[pk] == 0 || line[pk] == '\n') die("Too few arguments.\n> %s", line+start);

	// Do the same again to find the name of the function
	int pf = pk;
	while (line[pf] != ' ' && line[pf] != '\t' && line[pf] != '\n' && line[pf] != 0) pf++;
	if (line[pf] == 0 || line[pf] == '\n') die("Too few arguments.\n> %s", line+start);
	while (line[pf] == ' ' || line[pf] == '\t') pf++;
	if (line[pf] == 0 || line[pf] == '\n') die("Too few arguments.\n> %s", line+start);

	// Find the length of the name of the function
	int lf = 0;
	while (line[pf+lf] != ' ' && line[pf+lf] != '\t' && line[pf+lf] != '\n' && line[pf+lf] != 0) lf++;

	// Temporarily set a null byte after the function name so that we can use strcmp
	char borrowed = line[pf+lf];
	line[pf+lf] = 0;

	// Search the arrays to find a matching function
	// Choose which linked list we will need to add this too.
	fDict f = {NULL, NULL, 0};
	KeyCombo** comboList;
	for (int i = 0; i < RF_COUNT; i++)
		if (strcmp(line + pf, rootFunctions[i].name) == 0) {
			f = rootFunctions[i];
			comboList = &rootKeyCombos;
			break;
		}
	if (f.name == NULL) for (int i = 0; i < CF_COUNT; i++)
		if (strcmp(line + pf, clientFunctions[i].name) == 0) {
			f = clientFunctions[i];
			comboList = &clientKeyCombos;
			break;
		}
	
	// If no function was found, then die
	if (f.name == NULL) die("Invalid function name \"%s\".", line + pf);
	// Restore the character after the function name
	line[pf+lf] = borrowed;

	// Determine the key combination
	unsigned int mask = 0;
	int keycode = 0;

	int ppk = pk; // Preserve for die
	int p = pk;
	do {
		p++;
		// If we are looking at a whitespace or a +, then work with the last part
		if (line[p] == '+' || line[p] == ' ' || line[p] == '\t') {
			// Determine the number of characters in this token
			char b = 0;
			switch(p-pk) {
				case 5:
					// This could be 'space' or 'shift' or 'enter'
					if (memcmp(line + pk, "shift", 5) == 0) {
						mask |= ShiftMask;
						break;
					}
					// If enter, then just call with the XK_Retern value
					if (memcmp(line + pk, "enter", 5) == 0) {
						if (keycode != 0) die("Only one symbol allowed in a key combination.");
						keycode = (int)XKeysymToKeycode(display, XK_Return);
						if (keycode == NoSymbol) die("No symbol matched to \'enter\'.");
						break;
					}
					if (memcmp(line + pk, "space", 5) != 0) {
						// Place a null byte for the convenience of die
						line[p] = 0;
						die("Unknown mask \"%s\".", line + pk);
					}
					// We can replace the character at line[pk] sneakily with a space and reuse the
					// code below.
					b = line[pk];
					line[pk] = ' ';
				case 1: {
					// If there is only one character, then this is a letter/symbol
					// There can only be one in a key-combination, die if keycode is already set
					if (keycode != 0) die("Only one symbol allowed in a key combination.");

					// Get the Keysym from the character.
					char keystring[2] = {line[pk], 0};
					KeySym ks = XStringToKeysym(keystring);
					keycode = (int)XKeysymToKeycode(display, ks);
					// If XKeysymToKeycode fails, it will return NoSymbol.
					if (keycode == NoSymbol) die("No symbol matched to \'%c\'.", line[pk]);

					// If we had a space, then we must replace the character we swapped out
					if (b != 0) line[pk] = b;
					break;
				}
				case 3: {
					// This could be 'win' or 'alt'
					if (memcmp(line + pk, "win", 3) == 0) mask |= Mod4Mask;
					else if (memcmp(line + pk, "alt", 3) == 0) mask |= Mod1Mask;
					else {
						line[p] = 0;
						die("Unknown mask \"%s\".", line + pk);
					}
					break;
				}
				case 4: {
					// This could be 'ctrl'
					if (memcmp(line + pk, "ctrl", 4) == 0) mask |= ControlMask;
					else {
						line[p] = 0;
						die("Unknown mask \"%s\".", line + pk);
					}
					break;
				}
				default:
					// If the key doesn't match, then die
					line[p] = 0;
					die("Unknown mask \"%s\".", line + pk);
			}

			// Reset pk and p to look at the next key/mask
			// If we are on a plus, then there must be another thing to look at
			if (line[p] == '+') pk = ++p;
			else break;
		}
	} while (line[p] != ' ' && line[p] != '\t');

	// If keycode was not set, then die
	if (keycode == 0) {
		line[p] = 0;
		die("Invalid key combination \"%s\"\n At least one printable charater is needed.", line+ppk);
	}

	// Determine whether there is an argument given
	int pa = pf + lf;
	bool hasArg = true;
	while (line[pa] == ' ' || line[pa] == '\t') pa++;
	if (line[pa] == '\n' || line[pa] == 0) hasArg = false;

	if (f.needArg == 0 && hasArg) die("No argument required for function \"%s\"\n> %s", f.name, line);
	if (f.needArg != 0 && !hasArg) die("Argument required for function \"%s\"\n> %s", f.name, line);

	void* argument = NULL;

	// If an argument is needed, then extract it
	if (f.needArg == FDICT_NEEDINT) {
		// Check that only integers are in the argument
		int la = 0;
		while (line[pa+la] >= '0' && line[pa+la] <= '9') la++;
		// If it was a non-whitespace character which stopped this number
		if (line[pa+la] != 0 && line[pa+la] != '\n' && line[pa+la] != ' ') {
			while (line[la+pa] != 0 && line[la+pa] != ' ' && line[la+pa] != '\n' && line[la+pa] != '\t') la++;
			line[pa+la] = 0;
			die("The argument given \"%s\" is not an integer.", line + pa);
		}
		line[pa+la] = 0;
		#pragma GCC diagnostic ignored "-Wint-to-pointer-cast" // This is dodgy
		argument = (void*)atoi(line + pa);
		#pragma GCC diagnostic pop
		
	} else if (f.needArg == FDICT_NEEDSTRING) {
		// Find where the argument ends
		int la = 0;
		while (line[pa+la] != 0 && line[pa+la] != '\n') la++;
		// Allocate some memory for the string
		argument = malloc(la + 1);
		// Copy in the string argument
		memcpy(argument, line+pa, la);
		((char*)argument)[la] = 0;
	}

	// Create the structure
	KeyCombo* newCombo = calloc(sizeof(KeyCombo), 1);
	newCombo->modifiers = mask;
	newCombo->keycode = keycode;
	newCombo->function = f.function;
	newCombo->arg = argument;
	newCombo->hasArg = f.needArg;

	// Add the structure to the linked list
	if (*comboList == NULL) {
		*comboList = newCombo;
		return;
	}

	KeyCombo* list = *comboList;
	while (list->next != NULL) list = list->next;
	list->next = newCombo;
}