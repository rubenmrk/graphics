# Files to compile
_TARGET = ui
_OBJECTS = main.o application/application.o gl3w.o
PLATFORM = wayland

# Converter files
_CONVTARGET = obj2msh
_CONVOBJECTS = converter.o

# Includes, libraries, preprocessor
LIBS = -lpthread -lfreetype
INCLUDE = -Isrc -Iinclude -I/usr/include/freetype2
PREPROC =

# Platform specific
_WOBJECTS = wayland-protocol/xdg-shell.o wayland-protocol/pointer-constraints-v1.o wayland-protocol/relative-pointer-v1.o
WLIBS = -lwayland-client -lwayland-egl -lEGL
WPREPROC = -DUSE_WAYLAND

# Add the platform specific part
ifeq ($(PLATFORM), wayland)
	_OBJECTS := $(_OBJECTS) $(_WOBJECTS)
	LIBS := $(LIBS) $(WLIBS)
	PREPROC	:= $(PREPROC) $(WPREPROC)
endif

# Compile options
CCX = clang++
CC = clang
CFLAGS = -Wall -pedantic -O3 -g
CXFLAGS = -std=c++17 $(CFLAGS)
CCFLAGS = -std=c17 $(CFLAGS)

# The directories where to find the source files
BIN = ./bin/
SRC = ./src/

# Recompile everything whenever a core header changes
BASE = $(wildcard $(SRC)base/*.hpp)
GFX = $(wildcard $(SRC)opengl/*.hpp)
APP = $(wildcard $(SRC)application/*.hpp)

# Path to files
OBJECTS = $(addprefix $(BIN), $(_OBJECTS))
TARGET = $(addprefix $(BIN), $(_TARGET))
CONVOBJECTS = $(addprefix $(BIN), $(_CONVOBJECTS))
CONVTARGET = $(addprefix $(BIN), $(_CONVTARGET))

.DEFAULT_GOAL = all

# C rule
$(BIN)%.o: $(SRC)%.c
	$(CC) $(CCFLAGS) $(INCLUDE) $(PREPROC) -c $< -o $@

# C++ rule
$(BIN)%.o: $(SRC)%.cpp ${BASE} ${GFX}
	$(CCX) $(CXFLAGS) $(INCLUDE) $(PREPROC) -c $< -o $@

# Rule for application.cpp
$(BIN)application/application%.o: $(SRC)application/application%.o ${BASE} ${GFX} ${APP}
	$(CCX) $(CXFLAGS) $(INCLUDE) $(PREPROC) -c $< -o $@

$(BIN)$(_CONVOBJECTS)%.o : $(SRC)$(_CONVOBJECTS).cpp

# Rule to build executables
$(TARGET): $(OBJECTS)
	$(CCX) -o $(TARGET) $(OBJECTS) $(LIBS)

$(CONVTARGET): $(CONVOBJECTS)
	$(CCX) -o $(CONVTARGET) $(CONVOBJECTS)

.PHONY: conv
conv: $(CONVTARGET)

.PHONY: all
all: $(TARGET) $(CONVTARGET)

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJECTS) $(CONVTARGET) $(CONVOBJECTS)
