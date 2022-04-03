#include "sxwm.h"
#include "util.h"

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

char* shmName = NULL;
int shmFd;
SXWMData* sxwmData = NULL;

int createShared(int write)
{
	// Get the display name from the environment variable
	char* envDisplay = getenv("DISPLAY");
	if (envDisplay == NULL) die("Could not determine display name.");
	int envDisplayLen = strlen(envDisplay);
	if (envDisplayLen <= 1) die("Invalid display name: %s", envDisplay);
	
	// Determine the name of the shm file
	shmName = malloc(envDisplayLen + 5);
	if (shmName == NULL) die("Could not allocate memory.");
	memcpy(shmName, "/sxwm", 5);
	memcpy(shmName+5, envDisplay+1, envDisplayLen);

	// Open the shared memory
	if (write)
		shmFd = shm_open(shmName, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	else
		shmFd = shm_open(shmName, O_RDONLY | O_CREAT, S_IRUSR);
	if (shmFd == -1) die("Could not create shared memory.");

	return shmFd;
}

void mapShared(int write)
{
	if (write)
		sxwmData = mmap(NULL, sizeof(SXWMData), PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
	else
		sxwmData = mmap(NULL, sizeof(SXWMData), PROT_READ, MAP_SHARED, shmFd, 0);

	if (sxwmData == MAP_FAILED)
		die("Could not create shared memory.");
}