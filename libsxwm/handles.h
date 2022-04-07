#pragma once

typedef int SXWMHandle;

#define SXWM_HANDLE_SOCKET (1)

union sxwm_handle {
	struct {
		SXWMHandle handle;
		int type;
	} any;
	struct {
		SXWMHandle handle;
		int type;
		int fd;
	} socket;
};

const union sxwm_handle *SXWMGetHandle(SXWMHandle);
const union sxwm_handle *SXWMRegisterHandle(const union sxwm_handle*);
int SXWMRemoveHandle(SXWMHandle);