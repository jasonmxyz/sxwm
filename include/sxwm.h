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

struct sxwm_monitor_spec_item {
	int nameoffset;
	int id;
	int x;
	int y;
	int width;
	int height;
};

struct sxwm_monitor_spec {
	int nmonitors;
	struct sxwm_monitor_spec_item monitors[];
};

struct sxwm_monitor_spec *SXWMGetMonitors(SXWMHandle handle);