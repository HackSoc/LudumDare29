OBJDIR=objs

CC=clang
CFLAGS=-c -Wall -Wextra -pedantic -g -std=c99
LDFLAGS=
SOURCES=ld29.c

OBJECTS=$(addprefix $(OBJDIR)/,$(SOURCES:.c=.o))
TARGET=ld29

all: $(TARGET)
	
$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

$(OBJDIR)/%.o: %.c
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

gdbrun: $(TARGET)
	gdb $(TARGET)
