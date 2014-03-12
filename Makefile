CC		:= clang
PKGS	:= libspotify
CFLAGS	:= -std=gnu99 -ggdb -Wall `pkg-config --cflags $(PKGS)`
LDFLAGS	:= `pkg-config --libs $(PKGS)` -lpthread

TARGET	:= spoticli
SRCDIR	:= src
OBJDIR	:= build
SOURCES := $(shell find $(SRCDIR)/ -type f -name '*.c') appkey.c
OBJECTS := $(addprefix $(OBJDIR)/, $(patsubst $(SRCDIR)/%, %, $(SOURCES:.c=.o)))
DEPS	:= $(OBJECTS:.o=.deps)

VPATH	:= $(SRCDIR):.	# allows make to loop for deps


all:	full_clean $(TARGET)

$(TARGET):	$(OBJECTS)
	@echo "Linking $@..."
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

$(OBJDIR)/%.o:	%.c
	@mkdir -p $(OBJDIR)	# create build
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -MD -MF $(@:.o=.deps) -c -o $@ $<

clean:
	@echo "Cleaning..."
	$(RM) -r $(OBJDIR) $(TARGET)

full_clean:	clean
	@echo "Removing 'tmp/'..."
	$(RM) -r tmp/

-include $(DEPS)

.PHONY: clean full_clean
