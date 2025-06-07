CC = gcc
AR = ar

WARNINGS = -Werror -Wall -Wextra -Wstrict-prototypes -Wold-style-definition -Wpedantic -Wconversion -Wsign-conversion -Wshadow -Wpointer-arith -Wvla -Wcast-align -Wstrict-overflow -Wnested-externs -Wstrict-aliasing -Wredundant-decls -Winit-self -Wmissing-noreturn 

CFLAGS = $(WARNINGS) -fPIC -I./include -I./src/common -std=c99
OUTDIR = build
SOURCES = $(wildcard src/common/*.c)
PLATFORM = $(shell uname -s)

EXAMPLES_SOURCES = $(filter-out examples/gamepadapp.c, $(wildcard examples/*.c))
EXAMPLES  = $(EXAMPLES_SOURCES:.c=)

EXTRA_EXAMPLES = examples/gamepadapp

ifeq ($(CC), emcc)
	EXT = so
	LDFLAGS = -shared 
	LIBS = 
else ifeq ($(PLATFORM),Linux)
	EXT = so
	LDFLAGS = -shared 
	CFLAGS += -I./src/linux/ 
	LIBS = 

	RGFW_LIBS = -lX11 -lXrandr -lGL -lm 
	LIBEVDEV_SOURCES = $(wildcard src/linux/libevdev/*.c)
	LIBEVDEV_OBJECTS = $(LIBEVDEV_SOURCES:.c=.o)

	SOURCES += $(wildcard src/linux/*.c)
	SOURCES += $(LIBEVDEV_SOURCES) 
else ifeq ($(PLATFORM),Darwin)
	EXT = dylib
	LDFLAGS = -dynamiclib
	LIBS =  -framework IOKit -lm
	RGFW_LIBS = -framework IOKit -framework Cocoa 
	IMPLIB = $(OUTDIR)/libminigamepad.a
else
	PLATFORM = Windows
endif

ifeq ($(PLATFORM),Windows)
   	EXT = dll
	LDFLAGS = -shared 
	# change later
	BACKEND ?= windows
	
	RGFW_LIBS = -lopengl32 -lgdi32 -lm

	ifeq ($(BACKEND),windows)
		LIBS = 	
		CFLAGS += -I./src/windows/ 
		SOURCES += $(wildcard src/windows/*.c)	
	else ifeq ($(BACKEND),WINMM)
		LIBS = -lwinmm	
		CFLAGS += -I./src/winmm/ 
		SOURCES += $(wildcard src/winmm/*.c)
	endif

	CFLAGS += -D BUILD_LIBTYPE_SHARED
endif

TARGET = $(OUTDIR)/libminigamepad.$(EXT) \
		 $(OUTDIR)/libminigamepad.a

OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET) $(EXAMPLES) $(EXTRA_EXAMPLES) 
$(OUTDIR)/libminigamepad.a: $(OBJECTS) | $(OUTDIR)
	$(AR) rcs $@ $(OBJECTS)

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

examples/gamepadapp: examples/gamepadapp.c
	$(CC) -I./include $< $(LIBS) $(RGFW_LIBS) ./build/libminigamepad.a -o $(OUTDIR)/$@ 

$(EXAMPLES): %: %.c		
	$(CC) -static $(CFLAGS) -I. $< $(LIBS) -L./build -lminigamepad -o $(OUTDIR)/$@ 

$(LIBEVDEV_OBJECTS): %.o: %.c
	$(CC) -fPIC  -c $< -o $@

$(OUTDIR):
	mkdir -p $(OUTDIR)
	mkdir -p $(OUTDIR)/examples
	cp DejaVuSans.ttf ./$(OUTDIR)/examples

clean:
	echo $(EXT) $(BACKEND) $(LIBS)
	rm -rf $(OUTDIR) $(OBJECTS)

.PHONY: all clean
