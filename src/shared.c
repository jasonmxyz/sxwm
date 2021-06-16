#include "shared.h"
#include "util.h"

#include <sys/shm.h>
#include <sys/stat.h>

int sid;
Shared* shared = NULL;

// Create the shared memory segment and create references to the variables within for easy access
// in all parts of the project.
void createSharedMemory() {
	// The segment should be private, with read and write access for us.
	sid = shmget(IPC_PRIVATE, sizeof(Shared), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

	// -1 is returned on error
	if (sid == -1) die("Could not create shared memory segment.");
}

// Attach the currnet process to the shared memory segment with the id in `sid`.
void attachToSharedMemory() {
	// Attach to the segment
	Shared* s = shmat(sid, NULL, 0);

	// -1 is returned on error
	if (s == (Shared*)-1) die("Could not attach to shared memory segment.");

	// Update the global variables to reference the shared memory segment.
	shared = s;
}

// Detatch from the shared memory segment
void detatchFromSharedMemory() {
	shmdt(shared);
}

// Destroy the shared memory segment
void destroySharedMemory() {
    shmctl(sid, IPC_RMID, 0);
}