CC := gcc
CFLAGS := -std=c99 -g
LIBS := -lX11

FILES := main
OBJ := $(addsuffix .o, $(FILES) )
DEP := header.h Makefile

.PHONY: all clean

all: sxwm

clean:
	rm -f sxwm $(OBJ)

$(OBJ): %.o : %.c $(DEP)
	$(CC) -c -o $@ $< $(CFLAGS)

sxwm: % : $(OBJ)
	$(CC) -o $@ $(LIBS) $^ $(CFLAGS)
