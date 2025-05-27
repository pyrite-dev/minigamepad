CC = gcc
AR = ar

WARNINGS = -Werror -Wall -Wextra -Wstrict-prototypes -Wold-style-definition -Wpedantic -Wconversion -Wsign-conversion -Wshadow -Wpointer-arith -Wvla -Wcast-align -Wstrict-overflow -Wnested-externs -Wstrict-aliasing -Wredundant-decls -Winit-self -Wmissing-noreturn 

CFLAGS = $(WARNINGS) -fPIC -I./include
OUTDIR = build
SOURCES = $(wildcard src/common/*.c)
PLATFORM = $(shell uname -s)

EXAMPLES_SOURCES = $(wildcard examples/*.c)
EXAMPLES  = $(EXAMPLES_SOURCES:.c=)

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
	IMPLIB = $(OUTDIR)/libminigamepad.a
else
	EXT = dll
	LDFLAGS = -shared
	LIBS = 
	PLATFORM = Windows
endif

TARGET = $(OUTDIR)/libminigamepad.$(EXT) \
		 $(OUTDIR)/libminigamepad.a

OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET) $(EXAMPLES) 
$(OUTDIR)/libminigamepad.a: $(OBJECTS) | $(OUTDIR)
	$(AR) ar rcs $@ $(OBJECTS)

$(OUTDIR)/libminigamepad.$(EXT): $(OBJECTS) | $(OUTDIR)
ifeq ($(PLATFORM),Windows)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS) -Wl,--out-implib,$(OUTDIR)/libminigamepad.lib
else
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS)
endif

$(OUTDIR)/%.o: $(SOURCES) | $(OUTDIR)
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(EXAMPLES): %: %.c		
	$(CC) $(CFLAGS) -I. $< $(LIBS) -o $(OUTDIR)/$@ 

$(OUTDIR):
	mkdir -p $(OUTDIR)
	mkdir -p $(OUTDIR)/examples

clean:
	rm -rf $(OUTDIR) $(OBJECTS)

.PHONY: all clean
