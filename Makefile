CC = gcc
CFLAGS = -std=gnu11 -g -D VERBOSE
LIBS = -lX11
OBJDIR = obj
SRCDIR = src

FILES := main util clients handlers tile input bar settings control shared
HEADERS := util.h clients.h settings.h shared.h
OBJ := $(addprefix $(OBJDIR)/, $(addsuffix .o, $(FILES)))
DEP := $(addprefix $(SRCDIR)/, $(HEADERS)) Makefile

.PHONY: all clean objdir

all: sxwm

objdir:
	mkdir -p $(OBJDIR)

clean: objdir
	rm -f sxwm $(OBJDIR)/*

$(OBJ): $(OBJDIR)/%.o : $(SRCDIR)/%.c $(DEP) objdir
	$(CC) -c -o $@ $< $(CFLAGS)

sxwm: % : $(OBJ)
	$(CC) -o $@ $(LIBS) $^ $(CFLAGS)

test: sxwm
	xinit ./sxwm -c ./share/defaultconfig -- $$(which Xephyr) :1 -ac -br -screen 1280x720