#pragma once

/*
 * libsxwm provides functions used to communicate with the SXWM window manager.
 */

int SXWMDefaultSocketPath(char *buffer);
int SXWMConnectSocket(const char *path);