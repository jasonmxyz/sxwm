.PHONY: all clean

all: sxwm

clean:
	make -C src clean

sxwm:
	make -C src sxwm

test: sxwm
	xinit ./src/sxwm -c ./share/defaultconfig -- $$(which Xephyr) :1 -ac -br -screen 1280x720