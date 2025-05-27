CC = gcc
AR = ar

CFLAGS = -Wall -fPIC -I./include
OUTDIR = build
SOURCES = $(wildcard src/common/*.c)
PLATFORM = $(shell uname -s)

ifeq ($(PLATFORM),Linux)
   	EXT = so
	LDFLAGS = -shared 
	CFLAGS += -I./src/linux/ 
    LIBS = 
    TARGET = $(OUTDIR)/libminigamepad.$(EXT)
	SOURCES += $(wildcard src/linux/*.c)
	SOURCES += $(wildcard src/linux/*/*.c)
	OBJECTS = $(SOURCES:.c=.o)
else ifeq ($(PLATFORM),Darwin)
    EXT = dylib
    LDFLAGS = -dynamiclib
    LIBS =  -framework IOKit
    TARGET = $(OUTDIR)/libminigamepad.$(EXT)
    OBJECTS = $(SOURCES:.c=.o)
else
	EXT = dll
    LDFLAGS = -shared
    LIBS = 
    DLL_TARGET = $(OUTDIR)/libminigamepad.$(EXT)
    IMPLIB = $(OUTDIR)/libminigamepad.a
    TARGET = $(DLL_TARGET)
    OBJECTS = $(patsubst src/%.c,$(OUTDIR)/%.o,$(SOURCES))
    CFLAGS += -DBUILD_DLL
endif

TARGET += $(OUTDIR)/libminigamepad.a

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
