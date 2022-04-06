CC = gcc
CFLAGS = -std=gnu11 -g -fPIC
LIBS =
OBJDIR = ../obj/libsxwm
DEPDIR = ../dep/libsxwm

FILES = sockets

.PHONY: all clean

all: libsxwm.so

clean:
	@-rm -rf libsxwm.so $(OBJDIR) $(DEPDIR)

DEP := $(addprefix $(DEPDIR)/, $(addsuffix .d, $(FILE)))
-include $(DEP)

$(OBJDIR)/%.o : %.c
	@mkdir -p $(@D:$(OBJDIR)%=$(DEPDIR)%)
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -MM -MT $@ $< -MF $(@:$(OBJDIR)/%.o=$(DEPDIR)/%.d)
	$(CC) -c -o $@ $< $(CFLAGS)

libsxwm.so: % : $(addprefix $(OBJDIR)/, $(addsuffix .o, $(FILES)))
	$(CC) -o $@ -shared $^