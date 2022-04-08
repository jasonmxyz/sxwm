#pragma once

#include <inttypes.h>

/* Useful structs for encoding and decoding SXWM_GETMONITORS messages. */
struct sxwm_message_getmonitors_header {
	uint32_t nMonitors;
	uint32_t monitorDataOffset;
	uint32_t workspaceIDsOffset;
	uint32_t namesOffset;
};

struct sxwm_message_getmonitors_monitordata {
	uint32_t id;
	uint32_t nameOffset;
	uint32_t x;
	uint32_t y;
	uint32_t width;
	uint32_t height;
	uint32_t nWorkspaces;
	uint32_t selectedWorkspace;
	uint32_t workspacesOffset;
};