
CC = gcc
CFLAGS = -std=c23 -pthread -Wall -Wextra -g $(shell pkg-config --cflags sdl3)
LDFLAGS = $(shell pkg-config --libs sdl3)

SRCDIR := src
INCDIR := include
BUILDDIR := build
OBJDIR := $(BUILDDIR)/obj
BINDIR := $(BUILDDIR)/bin

TARGET := $(BINDIR)/chip8
SRCS := $(wildcard $(SRCDIR)/*.c)
OBJS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))

INCFLAGS := -I$(INCDIR)

.PHONY: all clean run dirs

all: dirs $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(INCFLAGS) -c -o $@ $<

dirs:
	mkdir -p $(OBJDIR) $(BINDIR)

clean:
	rm -rf $(BUILDDIR)/*

SUPPORT_ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
ifneq ($(SUPPORT_ARGS),)
$(eval $(SUPPORT_ARGS):;@:)
$(eval .PHONY: $(SUPPORT_ARGS))
endif

run: all
	./$(TARGET) $(SUPPORT_ARGS)


