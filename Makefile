OBJDIR=objs

CC=clang
CFLAGS=-c -Wall -Wextra -Werror -pedantic -g -std=c99
LDFLAGS=-lcurses -lm
SOURCES=$(wildcard *.c)

OBJECTS=$(addprefix $(OBJDIR)/,$(SOURCES:.c=.o))
TARGET=ld29

ifdef AUTOPLAY
CFLAGS += -DAUTOPLAY
endif

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

$(OBJDIR)/%.o: %.c
	mkdir -p $(OBJDIR)
	$(CC)  $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

gdbrun: $(TARGET)
	gdb $(TARGET)
