#pragma once

#include <X11/Xlib.h>

void configureRequest(XEvent e);
void mapRequest(XEvent e);
void unmapNotify(XEvent e);
int nothingHandler(Display* display, XErrorEvent* e);
int errorHandler(Display* display, XErrorEvent* e);