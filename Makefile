CC = gcc
AR = ar

CFLAGS = -Wall -fPIC -I./include
OUTDIR = build
SOURCES = $(wildcard src/common/*.c)
PLATFORM = $(shell uname -s)

ifeq ($(CC), emcc)
	EXT = so
	LDFLAGS = -shared 
    LIBS = 
else ifeq ($(PLATFORM),Linux)
   	EXT = so
	LDFLAGS = -shared 
	CFLAGS += -I./src/linux/ 
    LIBS = 
	SOURCES += $(wildcard src/linux/*.c)
	SOURCES += $(wildcard src/linux/*/*.c)
else ifeq ($(PLATFORM),Darwin)
    EXT = dylib
    LDFLAGS = -dynamiclib
    LIBS =  -framework IOKit
else
	EXT = dll
    LDFLAGS = -shared
    LIBS = 
endif

TARGET = $(OUTDIR)/libminigamepad.$(EXT) \
		 $(OUTDIR)/libminigamepad.a

OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET) 
$(OUTDIR)/libminigamepad.a: $(OBJECTS) | $(OUTDIR)
	$(AR) ar rcs $@ $(OBJECTS)

$(OUTDIR)/libminigamepad.$(EXT): $(OBJECTS) | $(OUTDIR)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS)

$(OUTDIR)/%.o: $(SOURCES) | $(OUTDIR)
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTDIR):
	mkdir -p $(OUTDIR)
	mkdir -p $(OUTDIR)/common
	mkdir -p $(OUTDIR)/linux

clean:
	rm -rf $(OUTDIR)
	rm -f source/*.o

.PHONY: all clean
