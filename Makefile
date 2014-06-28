CC		:= clang
PKGS	:= libspotify alsa ncurses form
CFLAGS	:= -std=gnu99 -ggdb -Wall `pkg-config --cflags $(PKGS)`
LDFLAGS	:= `pkg-config --libs $(PKGS)` -lpthread

TARGET	:= spoticli
SRCDIR	:= src/spoticli
OBJDIR	:= build
SOURCES := $(shell find $(SRCDIR)/ -type f -name '*.c') appkey.c
OBJECTS := $(addprefix $(OBJDIR)/, $(patsubst $(SRCDIR)/%, %, $(SOURCES:.c=.o)))
DEPS	:= $(OBJECTS:.o=.deps)

VPATH	:= $(SRCDIR):.	# allows make to loop for deps

define cc_command
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -I./src -MD -MF $(@:.o=.deps) -c -o $@ $<
endef

all:	full_clean $(TARGET)

$(TARGET):	$(OBJECTS)
	@echo "Linking $@..."
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

# handle appkey
$(OBJDIR)/appkey.o:	appkey.c
	@mkdir -p $(OBJDIR)
	$(cc_command)

$(OBJDIR)/%.o:	$(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)	# create build
	$(cc_command)

$(OBJDIR)/ui/%.o: $(SRCDIR)/ui/%.c
	@mkdir -p $(OBJDIR)/ui
	$(cc_command)

$(OBJDIR)/spotify/%.o:	$(SRCDIR)/spotify/%.c
	@mkdir -p $(OBJDIR)/spotify
	$(cc_command)

clean:
	@echo "Cleaning..."
	$(RM) -r $(OBJDIR) $(TARGET)

full_clean:	clean
	@echo "Removing 'tmp/'..."
	$(RM) -r tmp/

-include $(DEPS)

.PHONY: clean full_clean
