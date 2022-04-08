#pragma once

#include <inttypes.h>

/*
 * libsxwm provides functions used to communicate with the SXWM window manager.
 */

typedef int SXWMHandle;

int SXWMDefaultSocketPath(char *buffer);
SXWMHandle SXWMConnectSocket(const char *path);
int SXWMDisconnectSocket(SXWMHandle handle);
int SXWMSocketFD(SXWMHandle handle);

struct sxwm_header {
	uint32_t type;
	uint32_t size;
	uint32_t seq;
};

int SXWMSend(int socket, uint32_t type, uint32_t size, void *data);
int SXWMSendSeq(int socket, uint32_t type, uint32_t size, void *data, int seq);
void *SXWMRecieve(int socket, struct sxwm_header *header);

#define SXWM_IGNORE ((uint32_t)0)
#define SXWM_ECHO ((uint32_t)1)
#define SXWM_GETMONITORS ((uint32_t)2)
#define SXWM_MAX ((uint32_t)2)

struct sxwm_monitor {
	uint32_t id;
	char *name;
	uint32_t x;
	uint32_t y;
	uint32_t width;
	uint32_t height;
	uint32_t nWorkspaces;
	uint32_t selectedWorkspace;
	uint32_t *workspaces;
};

struct sxwm_monitor *SXWMGetMonitors(SXWMHandle handle, int *retN);