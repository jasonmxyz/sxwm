CC := gcc
CFLAGS := -std=c99 -g -D VERBOSE
LIBS := -lX11

FILES := main util clientList handlers
HEADERS := global.h clientList.h handlers.h
OBJ := $(addsuffix .o, $(FILES) )
DEP := $(HEADERS) Makefile

.PHONY: all clean

all: sxwm

clean:
	rm -f sxwm $(OBJ)

$(OBJ): %.o : %.c $(DEP)
	$(CC) -c -o $@ $< $(CFLAGS)

sxwm: % : $(OBJ)
	$(CC) -o $@ $(LIBS) $^ $(CFLAGS)

test: sxwm
	xinit ./sxwm -- $$(which Xephyr) :1 -ac -br -screen 800x600