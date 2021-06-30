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

CmdQueue* commandQueue = NULL;

// Arrays of bindable functions
extern void selectTag(int t);
extern void stop();
extern void runCmd(char* command);
extern void killFocusedWindow();
extern void toggleFloating();
fDict rootFunctions[] = {{"selectTag", selectTag, FDICT_NEEDINT},
						  {"exit", stop, 0},
						  {"run", runCmd, FDICT_NEEDSTRING},
						  {"killWindow", killFocusedWindow, 0},
						  {"toggleFloating", toggleFloating, 0},
						  {NULL, NULL, 0}};

void keybind(int argc, char** argv);
void doStmt(int argc, char** argv);

extern Display* display;

void readSettings(char* path) {
	// If the path is not set, then check in some predefined locations for it.
	// TODO
	if (path == NULL) die("No configuration file specified.");

	// Open the configuration file in read mode
	FILE* f = fopen(path, "r");
	if (!f) die("Could not open configuration file \"%s\".", path);

	int allocSize = 256;
	char* line = calloc(allocSize, 1);
	if (!line) die("Could not allocate memory.");

	// Read each line of the file until the end is reached.
	while (fgets(line, allocSize, f)) {
		// If the line was too long to fit in the allocated memory, then expand
		// it until the whole line can be read.
		while (line[allocSize - 2] != 0 && line[allocSize - 2] != '\n') {
			// Allocate more memory, and fill the extra space with zeros
			line = realloc(line, allocSize + 256);
			if (!line) die("Could not allocate memory.");
			memset(line + allocSize, 0, 256);

			// Read more of the line.
			fgets(line + allocSize - 1, 257, f);
			allocSize += 256;
		}

		// Generate argc and argv like variables for the line.
		int argc = 0;
		int argvSize = 5;
		char** argv = malloc(sizeof(char*) * argvSize);
		if (!argv) die("Could not allocate memory.");

		int inQuotes = 0;
		for (int i = 0; i < allocSize; i++) {
			// If we reach the end of the line, or the start of a comment, we
			// can finish with this line.
			if (line[i] == '\n' || line[i] == '#' || line[i] == 0) {
				line[i] = 0;
				break;
			}

			// If we encounter a '"', the toggle inQuotes and start/end an
			// argument.
			if (line[i] == '\"') {
				if (inQuotes) {
					line[i] = 0;
					inQuotes = 0;
					continue;
				} else {
					// If argv is full, then expand it.
					if (argc == argvSize) {
						argv = realloc(argv, sizeof(char*) * (argvSize + 5));
						if (!argv) die("Could not allocate memory.");
						argvSize += 5;
					}
					argv[argc++] = line + i + 1;
					inQuotes = 1;
				}
			}

			// If we encounter a printable character after a whitespace or at
			// the start of the line, then this is a new argument (if we aren't
			// in quotes).
			if (line[i] != ' ' && line[i] != '\t') {
				if (i == 0 || (line[i-1] == ' ' || line[i-1] == '\t' || line[i-1] == 0)) {
					if (!inQuotes) {
						// If argv is full, then expand it.
						if (argc == argvSize) {
							argv = realloc(argv, sizeof(char*) * (argvSize + 5));
							if (!argv) die("Could not allocate memory.");
							argvSize += 5;
						}
						argv[argc++] = line + i;
					}
				}
			}

			// If we encounter whitespace, and there wasn't whitespace
			// previously, then this is the end of an argument (if we aren't in
			// quotes).
			if (line[i] == ' ' || line[i] == '\t') {
				if (i > 0 && line[i-1] != ' ' && line[i-1] != '\t') {
					if (!inQuotes) {
						line[i] = 0;
					}
				}
			}
		}

		// Switch on the length of the operation to minimise the number of
		// comparisons needed
		if (argc > 0)
		switch (strlen(argv[0])) {
			case 3:
				if (memcmp(argv[0], "run", 3) == 0) doStmt(argc, argv);
				else die("Command not recognised \"%s\".", argv[0]);
				break;
			case 4:
				if (memcmp(argv[0], "bind", 4) == 0) keybind(argc, argv);
				else die("Command not recognised.");
				break;
			case 5:
				if (memcmp(argv[0], "start", 5) == 0) doStmt(argc, argv);
				else die("Command not recognised \"%s\".", argv[0]);
				break;
			default:
				die("Command not recognised \"%s\".", argv[0]);
		}

		// Get ready to read the next line
		free(argv);
		memset(line, 0, allocSize);
	}

	// Clean up
	free(line);
	fclose(f);
}

// Add a command to the queue to be run later.
void doStmt(int argc, char** argv) {
	// Check there are enough arguments
	if (argc <= 1) die("Not enough arguments for operation \"%s\".", argv[0]);

	// Find the lengths of all the strings
	int* lengths = malloc((argc-1) * sizeof(int));
	int total = 0;
	for (int i = 1; i < argc; i++) {
		int l = strlen(argv[i]);
		total += l;
		lengths[i-1] = l;
	}

	// Combine the strings
	char* combined = malloc(total + argc-1);
	if (!combined) die("Could not allocate memory.");

	int p = 0;
	for (int i = 1; i < argc; i++) {
		memcpy(combined + p, argv[i], lengths[i-1]);
		combined[p + lengths[i-1]] = ' ';
		p += lengths[i-1] + 1;
	}
	combined[p] = 0;

	// Create the CmdQueue object
	CmdQueue* new = malloc(sizeof(CmdQueue));
	new->cmd = combined;
	new->next = NULL;

	// Determine if the command should be run in a new session
	if (memcmp(argv[0], "run", 3) == 0) new->newSession = 0;
	else new->newSession = 1;

	// Add this command to the end of the queue
	CmdQueue** item = &commandQueue;
	while (*item) item = &((*item)->next);
	*item = new;

	free(lengths);
}

// Add a keybinding to the data structure
void keybind(int argc, char** argv) {
	// Check there are enough arguments
	if (argc < 3) die("Not enough arguments for operation \"%s\".", argv[0]);

	// Convert the written key binding into a keysym and mask
	unsigned int mask = 0;
	int keycode = 0;

	// Search argv[1] for words/symbols separated by '+'
	int start = 0;
	int arglen = strlen(argv[1]);
	for (int i = 0; i <= arglen; i++) {
		// If we find a plus, then the previous letters make up the name of the
		// key/mask
		if (argv[1][i] == '+' || argv[1][i] == 0) {
			if (i == 0) die("Badly formatted key combination: \"%s\".", argv[1]);
			// Switch on the length of the 'word' between the plusses
			switch (i - start) {
				// If the 'word' was only one character, then a key must have
				// been given.
				case 1: {
					// Only one letter can be in a keycode
					if (keycode != 0) die("Too many letter/symbols in \"%s\".", argv[1]);

					// Get the Keysym from the character.
					char keystring[2] = {argv[1][start], 0};
					KeySym ks = XStringToKeysym(keystring);
					keycode = (int)XKeysymToKeycode(display, ks);
					// If XKeysymToKeycode fails, it will return NoSymbol.
					if (keycode == NoSymbol) die("No symbol matched to \'%c\'.", argv[1][start]);
					break;
				}
				case 3: {
					// This could be 'win' or 'alt'
					if (memcmp(argv[1] + start, "win", 3) == 0) mask |= Mod4Mask;
					else if (memcmp(argv[1] + start, "alt", 3) == 0) mask |= Mod1Mask;
					else die("Unknown mask in \"%s\".", argv[1] + start);
					break;
				}
				case 4: {
					// This could be 'ctrl'
					if (memcmp(argv[1] + start, "ctrl", 4) == 0) mask |= ControlMask;
					else die("Unknown mask in \"%s\".", argv[1] + start);
					break;
				}
				case 5:
					// This could be 'space' or 'shift' or 'enter'
					if (memcmp(argv[1] + start, "shift", 5) == 0) mask |= ShiftMask;
					// If enter, then just call with the XK_Retern value
					else if (memcmp(argv[1] + start, "enter", 5) == 0) {
						if (keycode != 0) die("Too many letter/symbols in \"%s\".", argv[1]);
						keycode = (int)XKeysymToKeycode(display, XK_Return);
						if (keycode == NoSymbol) die("No symbol matched to \'enter\'.");
					}
					// If space, then just call with the XK_Space value
					else if (memcmp(argv[1] + start, "space", 5) == 0) {
						if (keycode != 0) die("Too many letter/symbols in \"%s\".", argv[1]);
						keycode = (int)XKeysymToKeycode(display, XK_space);
						if (keycode == NoSymbol) die("No symbol matched to \'enter\'.");
					}
					break;
				default:
					// If the key doesn't match, then die
					die("Could not interpret key combination: \"%s\".", argv[1]);
			}
			start = i+1;
		}
	}

	if (!keycode) die("No key was specified in \"%s\".", argv[1]);

	// Find the corresponding command in rootFunctions
	fDict command = {NULL, NULL, 0};
	for (int i = 0; rootFunctions[i].name != NULL; i++)
		if (strcmp(argv[2], rootFunctions[i].name) == 0) {
			command = rootFunctions[i];
			break;
		}
	if (!command.name) die("No command matches \"%s\".", argv[2]);

	// Check the correct number of arguments are given
	if (command.needArg == FDICT_NEEDINT && argc != 4)
		die("Command \"%s\" takes one integer argument.", argv[2]);
	if (command.needArg == FDICT_NEEDSTRING && argc < 4)
		die("Command \"%s\" requires an argument.", argv[2]);
	if (command.needArg == 0 && argc != 3)
		die("Command \"%s\" takes no arguments.", argv[2]);
	
	// Create the keycombo data structure.
	KeyCombo* combo = malloc(sizeof(KeyCombo));
	combo->modifiers = mask;
	combo->keycode = keycode;
	combo->function = command.function;
	combo->hasArg = command.needArg;

	// Generate the argument
	if (command.needArg == FDICT_NEEDINT) {
		// Scan the argument to make sure it is an integer
		int length = strlen(argv[3]);
		if ((argv[3][0] < '0' || argv[3][0] > '9') && argv[3][0] != '-')
				die("Argument \"%s\" is not an integer.", argv[3]);
		for (int i = 1; i < length; i++) {
			if (argv[3][i] < '0' || argv[3][i] > '9')
				die("Argument \"%s\" is not an integer.", argv[3]);
		}
		#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
		combo->arg = (void*)atoi(argv[3]);
		#pragma GCC diagnostic pop
	}
	else if (command.needArg == FDICT_NEEDSTRING) {
		// Find the lengths of the arguments, and concatenate them
		int* lengths = malloc(sizeof(int*) * argc - 3);
		int total = 0;
		for (int i = 3; i < argc; i++) {
			lengths[i-3] = strlen(argv[i]);
			total += lengths[i+3];
		}

		char* arg = malloc(total + argc - 3);
		int p = 0;
		for (int i = 3; i < argc; i++) {
			memcpy(arg + p, argv[i], lengths[i-3]);
			arg[p + lengths[i-3]] = ' ';
			p += lengths[i-3] + 1;
		}
		arg[p-1] = 0;

		combo->arg = (void*)arg;
		free(lengths);
	}

	// Add the keycode to the front of the list
	combo->next = rootKeyCombos;
	rootKeyCombos = combo;
}