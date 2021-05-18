CC := gcc
CFLAGS := -std=c99 -g -D VERBOSE
LIBS := -lX11

FILES := main util clientList handlers monitors tile input bar
HEADERS := util.h clientList.h handlers.h monitors.h tile.h input.h bar.h
OBJ := $(addsuffix .o, $(FILES) )
DEP := $(HEADERS) Makefile

.PHONY: all clean

all: sxwm

clean:
	make -C src clean

sxwm:
	make -C src sxwm

test: sxwm
	xinit ./src/sxwm -- $$(which Xephyr) :1 -ac -br -screen 1280x720