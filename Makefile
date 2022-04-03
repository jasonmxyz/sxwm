CC = gcc
CFLAGS = -std=gnu11 -g
LIBS = -lX11 -lrt
OBJDIR = obj
SRCDIR = src

# Code files for the bar and sxwm programs
BAR = bar util shared
SXWM = main clients handlers tile input settings control util shared

HEADERS := $(addprefix $(SRCDIR)/, util.h clients.h settings.h sxwm.h)

OBJ := $(addprefix $(OBJDIR)/, $(addsuffix .o, $(sort $(BAR) $(SXWM))))

.PHONY: all clean objdir

all: sxwm sxwmbar

objdir:
	mkdir -p $(OBJDIR)

clean: objdir
	rm -f sxwm sxwmbar $(OBJDIR)/*

$(OBJ): $(OBJDIR)/%.o : $(SRCDIR)/%.c $(HEADERS) objdir
	$(CC) -c -o $@ $< $(CFLAGS)

sxwm: % : $(addprefix $(OBJDIR)/, $(addsuffix .o, $(SXWM)))
	$(CC) -o $@ $(LIBS) $^ $(CFLAGS)

sxwmbar: % : $(addprefix $(OBJDIR)/, $(addsuffix .o, $(BAR)))
	$(CC) -o $@ $(LIBS) $^ $(CFLAGS)

test: sxwm sxwmbar
	xinit ./sxwm -c ./share/defaultconfig -- $$(which Xephyr) :1 -ac -br -screen 1280x720