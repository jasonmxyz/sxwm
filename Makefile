CC = gcc
CFLAGS = -std=gnu11 -g
DFLAGS = -D VERBOSE
LIBS = -lX11
OBJDIR = obj
SRCDIR = src

NORMAL := main clients handlers tile input bar settings control
DEBUG := util shared
HEADERS := $(addprefix $(SRCDIR)/, util.h clients.h settings.h shared.h)

NOBJ := $(addprefix $(OBJDIR)/, $(addsuffix .o, $(NORMAL)))
DOBJ := $(addprefix $(OBJDIR)/, $(addsuffix .o, $(DEBUG)))

.PHONY: all clean objdir

all: sxwm

objdir:
	mkdir -p $(OBJDIR)

clean: objdir
	rm -f sxwm $(OBJDIR)/*

$(NOBJ): $(OBJDIR)/%.o : $(SRCDIR)/%.c $(HEADERS) objdir
	$(CC) -c -o $@ $< $(CFLAGS)
$(DOBJ): $(OBJDIR)/%.o : $(SRCDIR)/%.c $(HEADERS) objdir
	$(CC) -c -o $@ $< $(CFLAGS) $(DFLAGS)

sxwm: % : $(NOBJ) $(DOBJ)
	$(CC) -o $@ $(LIBS) $^ $(CFLAGS)

test: sxwm
	xinit ./sxwm -c ./share/defaultconfig -- $$(which Xephyr) :1 -ac -br -screen 1280x720