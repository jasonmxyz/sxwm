#!/bin/sh

# Test SXWM in an X11 server using Xephyr.
# The script ./share/xrc is run to simulate some .xinitrc file, this executes
# SXWM.

# Usage:
# ./test.sh [OPTION ...]

xinit ./share/xrc $@ -- $(which Xephyr) :1 -ac -br -screen 1280x720