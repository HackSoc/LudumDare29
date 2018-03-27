OBJDIR=objs

CC=clang
CFLAGS=-c -Wall -Wextra -Werror -pedantic -g -std=c99
LDFLAGS=-lcurses -lm
SOURCES=$(wildcard *.c)
ifndef AUTOPLAY
SOURCES := $(filter-out autoplay.c,$(SOURCES))
endif

OBJECTS=$(addprefix $(OBJDIR)/,$(SOURCES:.c=.o))
TARGET=ld29

ifdef AUTOPLAY
CFLAGS += -DAUTOPLAY
endif

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

$(OBJDIR):
	mkdir -p $(OBJDIR)
	touch $(OBJDIR)

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) -o $@ $(CFLAGS) $<

clean:
	rm -f $(OBJECTS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

gdbrun: $(TARGET)
	gdb $(TARGET)
