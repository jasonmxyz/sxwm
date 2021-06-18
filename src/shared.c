#include "shared.h"
#include "util.h"

#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>

int size = 0;
void* location = 0;

// Create the shared memory segment and create references to the variables within for easy access
// in all parts of the project.
void createSharedMemory() {
	// Determine the size of memory to map. The smallest number of full pages which will fit the
	// Shared data structure.
	int s = (sizeof(Shared)/4096 + 1) * 4096;

	// Map the mempry and record the location/size
	location = mmap(NULL, s,PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	// -1 is returned on error
	if (location == (void*)-1) {
		printf("Could not allocate memory.\n");
		exit(1);
	}
}