CC = gcc
CFLAGS = -std=gnu11 -g
DFLAGS = -D VERBOSE
LIBS = -lX11 -lrt
OBJDIR = obj
SRCDIR = src

# Code files for the bar and sxwm programs
BAR = bar util shared
SXWM = main clients handlers tile input settings control util shared
# Which of the above files should be compiled with the debug flag?
DEBUG =

HEADERS := $(addprefix $(SRCDIR)/, util.h clients.h settings.h sxwm.h)

# Find the names of all object files which are to be compiled with or without the debug flag
DOBJ := $(addprefix $(OBJDIR)/, $(addsuffix .o, $(DEBUG)))
NOBJ := $(filter-out $(DOBJ), $(addprefix $(OBJDIR)/, $(addsuffix .o, $(sort $(BAR) $(SXWM)))))

.PHONY: all clean objdir

all: sxwm sxwmbar

objdir:
	mkdir -p $(OBJDIR)

clean: objdir
	rm -f sxwm bar $(OBJDIR)/*

$(NOBJ): $(OBJDIR)/%.o : $(SRCDIR)/%.c $(HEADERS) objdir
	$(CC) -c -o $@ $< $(CFLAGS)
$(DOBJ): $(OBJDIR)/%.o : $(SRCDIR)/%.c $(HEADERS) objdir
	$(CC) -c -o $@ $< $(CFLAGS) $(DFLAGS)

sxwm: % : $(addprefix $(OBJDIR)/, $(addsuffix .o, $(SXWM)))
	$(CC) -o $@ $(LIBS) $^ $(CFLAGS)

sxwmbar: % : $(addprefix $(OBJDIR)/, $(addsuffix .o, $(BAR)))
	$(CC) -o $@ $(LIBS) $^ $(CFLAGS)

test: sxwm sxwmbar
	xinit ./sxwm -c ./share/defaultconfig -- $$(which Xephyr) :1 -ac -br -screen 1280x720