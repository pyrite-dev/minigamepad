CC = gcc
AR = ar

WARNINGS = -Werror -Wall -Wextra -Wstrict-prototypes -Wold-style-definition -Wpedantic -Wconversion -Wsign-conversion -Wshadow -Wpointer-arith -Wvla -Wcast-align -Wstrict-overflow -Wnested-externs -Wstrict-aliasing -Wredundant-decls -Winit-self -Wmissing-noreturn 

CFLAGS = $(WARNINGS) -fPIC -std=c99
SOURCES = $(wildcard src/common/*.c)
PLATFORM = $(shell uname -s)

EXAMPLES_SOURCES = $(filter-out examples/rgfw_gamepad.c, $(wildcard examples/*.c))
EXAMPLES  = $(EXAMPLES_SOURCES:.c=)

EXTRA_EXAMPLES = examples/rgfw_gamepad

ifeq ($(CC), emcc)
	LIBS = 
else ifeq ($(PLATFORM),Linux)
	LIBS = 
	RGFW_LIBS = -lX11 -lXrandr -lGL -lm 
else ifeq ($(PLATFORM),Darwin)
	LIBS =  -framework IOKit -lm
	RGFW_LIBS = -framework IOKit -framework Cocoa -framework OpenGL 
else
	PLATFORM = Windows
endif

ifeq ($(PLATFORM),Windows)
	RGFW_LIBS = -lopengl32 -lgdi32 -lm
	LIBS = 	
endif

all: $(EXAMPLES) $(EXTRA_EXAMPLES) minigamepad.h 
examples/rgfw_gamepad: examples/rgfw_gamepad.c minigamepad.h 
ifneq ($(CC),emcc)
	$(CC) -I./ $< $(LIBS) $(RGFW_LIBS) -o $@ 
else
	@echo this example doesn not yet support WASM
endif

$(EXAMPLES): %: %.c minigamepad.h 
	$(CC) -std=c89 $(WARNINGS) -I. $< $(LIBS) -o $@

debug: all
	@for exe in $(EXAMPLES); do \
		echo "Running $$exe..."; \
		./$$exe; \
	done
	cd examples && ./rgfw_gamepad

clean:
	rm -rf $(EXAMPLES) examples/rgfw_gamepad
	
.PHONY: all clean
