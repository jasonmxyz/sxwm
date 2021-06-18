#include "memory.h"
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
		DEBUG("Page: size:     %d\n", pageInfo->size);

		// Try to allocate to each section in this page
		// Exit the loop SMallocInfo structure is outside of the current page
		for (SMallocInfo* mallocInfo = (SMallocInfo*)((void*)pageInfo + sizeof(PageInfo));
			 mallocInfo < (SMallocInfo*)((void*)pageInfo + pageInfo->size);
			 mallocInfo = (SMallocInfo*)((void*)mallocInfo + sizeof(SMallocInfo) + mallocInfo->size)) {
			DEBUG("Section: location:  %d", mallocInfo);
			DEBUG("Section: size:      %d", mallocInfo->size);
			DEBUG("Section: prev:      %d", mallocInfo->prev);
			DEBUG("Section: available: %d\n", mallocInfo->available);

			// If this section is available, and is big enough, then use it
			if (mallocInfo->available && mallocInfo->size >= length) {
				mallocInfo->available = false;

				// If it is possible to have another malloc between this one and
				// the next, then create another structure after this one.
				if (mallocInfo->size >= length + sizeof(SMallocInfo) + sizeof(int)) {
					SMallocInfo* next = (SMallocInfo*)((void*)mallocInfo + sizeof(SMallocInfo) + length);
					DEBUG("Createing new SMallocInfo at %d", next);
					next->size = mallocInfo->size - length - sizeof(SMallocInfo);
					next->available = true;
					next->prev = mallocInfo;
					DEBUG("                   with size %d", next->size);
					DEBUG("                 previous is %d\n", next->prev);
					mallocInfo->size = length;

					// Update the next SMallocInfo if it exists
					// The structure exists if the page is big enough for it to
					// have been made previously.
					if (mallocInfo + mallocInfo->size + sizeof(SMallocInfo) < (SMallocInfo*)((void*)pageInfo + pageInfo->size)) {
						((SMallocInfo*)(mallocInfo + sizeof(SMallocInfo) + mallocInfo->size))->prev = next;
					}
				}

				return ((void*)mallocInfo + sizeof(SMallocInfo));
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

// Free up some memory allocated in the pages
void sfree(void* addr) {
	DEBUG("sfree address: %d", addr);

	// Look in each page
	PageInfo* pageInfo = location;
	for (PageInfo* pageInfo = location; pageInfo != NULL; pageInfo = pageInfo->next) {
		DEBUG("Page location: %d", pageInfo);
		DEBUG("Page size:     %d", pageInfo->size);
		// If the address is inside this page, then try and free it,
		// otherwise, look at the next page
		if (addr > (void*)pageInfo && addr < (void*)pageInfo + pageInfo->size) {
			// Find the section which has this address
			for (SMallocInfo* mallocInfo = (SMallocInfo*)((void*)pageInfo + sizeof(PageInfo));
				 (void*)mallocInfo < addr;
				 mallocInfo = (SMallocInfo*)((void*)mallocInfo + sizeof(SMallocInfo) + mallocInfo->size)) {
				DEBUG("Section: location:  %d", mallocInfo);
				DEBUG("Section: size:      %d", mallocInfo->size);
				DEBUG("Section: prev:      %d", mallocInfo->prev);
				DEBUG("Section: available: %d", mallocInfo->available);
				// If the address of the start of this section matches, and
				// this section is not already free, then we can free things.
				if (addr == (void*)mallocInfo + sizeof(SMallocInfo) && !mallocInfo->available) {
					mallocInfo->available = true;
					DEBUG("Freeing");

					// If the section to the right is available, then merge them
					if ((void*)mallocInfo + 2*sizeof(SMallocInfo) + mallocInfo->size + sizeof(int) <= (void*)pageInfo + pageInfo->size) {
						SMallocInfo* right = (SMallocInfo*)((void*)mallocInfo + sizeof(SMallocInfo) + mallocInfo->size);
						if (right->available) {
							DEBUG("Merging with right");
							mallocInfo->size += sizeof(SMallocInfo) + right->size;
							// If there is another section after the one to the
							// right, then we need to update it's prev to point
							// to this section
							if ((void*)right + right->size + sizeof(SMallocInfo) < (void*)pageInfo + pageInfo->size) {
								((SMallocInfo*)((void*)right + sizeof(SMallocInfo) + right->size))->prev = mallocInfo;
							}
						}
					}

					// If the section to the left is available
					if (mallocInfo->prev != NULL && mallocInfo->prev->available) {
						DEBUG("Merging with left");
						mallocInfo->prev->size += sizeof(SMallocInfo) + mallocInfo->size;
						// The section to the right of this one (if it exists)
						// must now point to the previous section
						if ((void*)mallocInfo + mallocInfo->size + sizeof(SMallocInfo) < ((void*)pageInfo + pageInfo->size)) {
							((SMallocInfo*)((void*)mallocInfo + sizeof(SMallocInfo) + mallocInfo->size))->prev = mallocInfo->prev;
						}
					}
					
					// Done
					return;
				}
			}

			// We dont need to look further
			return;
		}
	}
}