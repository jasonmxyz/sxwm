#include "shared.h"
#include "util.h"

#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void* location = 0;

// Create the shared memory segment with mmap
void createSharedMemory() {
	// Determine the pagesize
	int pagesize = sysconf(_SC_PAGESIZE);
	DEBUG("Pagesize: %d", pagesize);
	if (pagesize == -1) {
		printf("Could not determine pagesize.\n");
		exit(1);
	}

	// Map the memory and record the location
	location = mmap(NULL, pagesize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	DEBUG("Location: %d", location);
	if (location == (void*)-1) {
		printf("Could not allocate memory.\n");
		exit(1);
	}

	// Setup the PageInfo and SMallocInfo
	PageInfo* pageInfo = location;
	SMallocInfo* mallocInfo = location + sizeof(PageInfo);
	pageInfo->size = pagesize;
	pageInfo->prev = NULL;
	pageInfo->next = NULL;
	mallocInfo->size = pageInfo->size - sizeof(PageInfo) - sizeof(mallocInfo);
	mallocInfo->available = true;
	mallocInfo->prev = NULL;
}

// Allocate some memory in the shared pages and return a pointer to it.
void* smalloc(int length) {
	// Round up the length to a multiple of the word size for performance
	length = length % sizeof(int) == 0 ? length
									   : (length / sizeof(int) + 1) * sizeof(int);
	DEBUG("smalloc length: %d", length);
	PageInfo* pageInfo = NULL;
	
	// Try and allocate some memory in each of the pages
	while (true) {
		// Get the next pageInfo structure
		if (pageInfo == NULL) {
			// If this is the first page
			pageInfo = location;
		} else if (pageInfo->next != NULL) {
			// Typically use the next page
			pageInfo = pageInfo->next;
		} else {
			// Create a new page
			// Determine an integer multiple of the page size which is big enough
			int size = sysconf(_SC_PAGESIZE);
			if (size == -1)
				die("Could not determine pagesize.");
			size = length + sizeof(PageInfo) + sizeof(SMallocInfo) <= size ? size
				 : ((length + sizeof(PageInfo) + sizeof(SMallocInfo)) / size + 1) * size;
			DEBUG("Creating new page");
			DEBUG("Size: %d", size);

			// Map the memory and record the location
			pageInfo->next = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
			DEBUG("Location: %d", location);
			if (location == (void*)-1)
				die("Could not allocate memory.");

			// Setup the next PageInfo
			pageInfo->next->size = size;
			pageInfo->next->prev = pageInfo;
			pageInfo->next->next = NULL;
			// And the initial SMallocInfo
			SMallocInfo* mallocInfo = (SMallocInfo*)((pageInfo->next) + sizeof(PageInfo));
			mallocInfo->size = pageInfo->next->size - sizeof(PageInfo) - sizeof(mallocInfo);
			mallocInfo->available = true;
			mallocInfo->prev = NULL;

			pageInfo = pageInfo->next;
		}
		DEBUG("Page: location: %d", pageInfo);
		DEBUG("Page: size:     %d", pageInfo->size);

		// Try to allocate to each section in this page
		// Exit the loop SMallocInfo structure is outside of the current page
		for (SMallocInfo* mallocInfo = (SMallocInfo*)((void*)pageInfo + sizeof(PageInfo));
			 mallocInfo < (SMallocInfo*)((void*)pageInfo + pageInfo->size);
			 mallocInfo = mallocInfo + sizeof(SMallocInfo) + mallocInfo->size) {
			DEBUG("Section: location:  %d", mallocInfo);
			DEBUG("Section: size:      %d", mallocInfo->size);
			DEBUG("Section: prev:      %d", mallocInfo->prev);
			DEBUG("Section: available: %d", mallocInfo->available);

			// If this section is available, and is big enough, then use it
			if (mallocInfo->available && mallocInfo->size >= length) {
				mallocInfo->available = false;

				// If it is possible to have another malloc between this one and
				// the next, then create another structure after this one.
				if (mallocInfo->size >= length + sizeof(SMallocInfo) + sizeof(int)) {
					SMallocInfo* next = mallocInfo + sizeof(SMallocInfo) + length;
					DEBUG("Createing new SMallocInfo at %d", next);
					next->size = mallocInfo->size - length - sizeof(SMallocInfo);
					next->available = true;
					next->prev = mallocInfo;
					DEBUG("                   with size %d", next->size);
					DEBUG("                 previous is %d", next->prev);
					mallocInfo->size = length;

					// Update the next SMallocInfo if it exists
					// The structure exists if the page is big enough for it to
					// have been made previously.
					if (mallocInfo + mallocInfo->size + sizeof(SMallocInfo) < (SMallocInfo*)((void*)pageInfo + pageInfo->size)) {
						((SMallocInfo*)(mallocInfo + sizeof(SMallocInfo) + mallocInfo->size))->prev = next;
					}
				}

				return mallocInfo + sizeof(SMallocInfo);
			}
		}
	}

	return NULL;
}

// Allocate some memory in the shared pages, zero the memory and return a pointer to it.
void* scalloc(int length) {
	void* mem = smalloc(length);

	if (mem == NULL) return NULL;

	memset(mem, 0, length);

	return mem;
}