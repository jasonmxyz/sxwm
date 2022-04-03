CC = gcc
CFLAGS = -std=gnu11 -g
LIBS = -lX11 -lrt
OBJDIR = obj
SRCDIR = src
DEPDIR = dep

# Code files for the bar and sxwm programs
BAR = bar util shared
SXWM = main clients handlers tile input settings control util shared

.PHONY: all clean

all: sxwm sxwmbar

clean:
	@-rm -rf sxwm sxwmbar $(OBJDIR) $(DEPDIR)

DEP := $(addprefix $(DEPDIR)/, $(addsuffix .d, $(sort $(BAR) $(SXWM))))
-include $(DEP)

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	@mkdir -p $(@D:$(OBJDIR)%=$(DEPDIR)%)
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -MM -MT $@ $< -MF $(@:$(OBJDIR)/%.o=$(DEPDIR)/%.d)
	$(CC) -c -o $@ $< $(CFLAGS)

sxwm: % : $(addprefix $(OBJDIR)/, $(addsuffix .o, $(SXWM)))
	$(CC) -o $@ $(LIBS) $^ $(CFLAGS)

sxwmbar: % : $(addprefix $(OBJDIR)/, $(addsuffix .o, $(BAR)))
	$(CC) -o $@ $(LIBS) $^ $(CFLAGS)

test: sxwm sxwmbar
	xinit ./sxwm -c ./share/defaultconfig -- $$(which Xephyr) :1 -ac -br -screen 1280x720