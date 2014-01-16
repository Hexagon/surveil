TARGET = surveil
LIBS = -lpng
CC = gcc
CFLAGS = -g -O2
INST_PREFIX = /usr/local

.PHONY: default all clean install

default: $(TARGET)
all: default

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)

  INST_PREFIX=/usr/local
    
install: $(TARGET)
	install -m 0755 $(TARGET) $(INST_PREFIX)/bin
