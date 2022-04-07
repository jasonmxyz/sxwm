CC = gcc
CFLAGS = -Iinclude -std=gnu11 -g
LIBS = -Llibsxwm -lX11 -lXrandr -lsxwm
OBJDIR = obj
SRCDIR = src
DEPDIR = dep

# Code files for the bar and sxwm programs
BAR = bar util shared
SXWM = main clients handlers input settings control util shared monitors ipc workspaces/workspaces workspaces/tiling/tiling workspaces/tiling/clients frames/frames frames/basic/basic

.PHONY: all clean

all: libsxwm/libsxwm.so sxwm sxwmbar

clean:
	@cd libsxwm; make clean
	@-rm -rf sxwm sxwmbar $(OBJDIR) $(DEPDIR)

force:
	@true

DEP := $(addprefix $(DEPDIR)/, $(addsuffix .d, $(sort $(BAR) $(SXWM))))
-include $(DEP)

libsxwm/libsxwm.so: force
	@cd libsxwm; $(MAKE) $(MFLAGS)

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	@mkdir -p $(@D:$(OBJDIR)%=$(DEPDIR)%)
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -MM -MT $@ $< -MF $(@:$(OBJDIR)/%.o=$(DEPDIR)/%.d)
	$(CC) -c -o $@ $< $(CFLAGS)

sxwm: % : $(addprefix $(OBJDIR)/, $(addsuffix .o, $(SXWM)))
	$(CC) -o $@ $(LIBS) $^ $(CFLAGS)

sxwmbar: % : $(addprefix $(OBJDIR)/, $(addsuffix .o, $(BAR)))
	$(CC) -o $@ $(LIBS) $^ $(CFLAGS)