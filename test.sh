#!/bin/sh

# Test SXWM in an X11 server using Xephyr.
# The script ./share/xrc is run to simulate some .xinitrc file, this executes
# SXWM.

# Usage:
# ./test.sh [OPTION ...]

export LD_LIBRARY_PATH=libsxwm:$LD_LIBRARY_PATH

xinit ./share/xrc $@ -- $(which Xephyr) :1 -screen 1600x600+0+0 +extension RANDR