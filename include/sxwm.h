#pragma once

#include <inttypes.h>

/*
 * libsxwm provides functions used to communicate with the SXWM window manager.
 */

int SXWMDefaultSocketPath(char *buffer);
int SXWMConnectSocket(const char *path);

struct sxwm_header {
	uint32_t type;
	uint32_t size;
	uint32_t seq;
};

int SXWMSend(int socket, uint32_t type, uint32_t size, void *data);
void *SXWMRecieve(int socket, struct sxwm_header *header);

#define SXWM_IGNORE ((uint32_t)0)
#define SXWM_ECHO ((uint32_t)1)
#define SXWM_MAX ((uint32_t)1)