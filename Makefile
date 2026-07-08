#
# 'make'        build executable file 'main'
# 'make clean'  removes all .o and executable files
#

# define the Cpp compiler to use
CXX = g++

# Build type: Debug or Release (override with `make BUILD_TYPE=Release`)
BUILD_TYPE ?= Debug

# define any compile-time flags
CXXFLAGS	:= -std=c++17 -Wall -Wextra -finput-charset=UTF-8 -fexec-charset=UTF-8
ifeq ($(BUILD_TYPE),Release)
	CXXFLAGS += -O3 -DNDEBUG
else
	CXXFLAGS += -g -O0
endif

# define library paths in addition to /usr/lib
#   if I wanted to include libraries not in /usr/lib I'd specify
#   their path using -Lpath, something like:
LFLAGS = 

# define output directory
OUTPUT	:= output
BUILD_DIR := build

# define source directory
SRC		:= src

# define include directory
INCLUDE	:= include

# define lib directory
LIB		:= lib

# Windows-specific settings
ifeq ($(OS),Windows_NT)
# Prefer 32-bit MinGW when bundled libs are 32-bit
ifneq ($(wildcard C:/MinGW/bin/g++.exe),)
	ifeq ($(CXX),g++)
		CXX := C:/MinGW/bin/g++.exe
	endif
endif
# Add mingw helper libs and winsock; ensure mingw libs come before GLFW
GLFW_DLL_IMPORT := lib/libglfw3dll.a
ifneq ($(wildcard $(GLFW_DLL_IMPORT)),)
	# Prefer the import library for the GLFW DLL to avoid static linking issues
	GLFW_LINK := $(GLFW_DLL_IMPORT)
else
	GLFW_LINK := -lglfw3
endif

LIBRARIES	:= $(GLFW_LINK) -lmingwex -lws2_32 -lopengl32 -lgdi32 -lwinmm -luser32 -lkernel32 -static-libgcc -static-libstdc++
MAIN	:= main.exe
# include main directory so main.cpp is compiled
SOURCEDIRS	:= $(SRC) $(SRC)/main $(SRC)/core $(SRC)/game $(SRC)/audio $(SRC)/graphics $(SRC)/geometry $(SRC)/third_party
# GLM repo layout here is include/glm/glm.hpp (nested glm/) so add include/glm to search path
INCLUDEDIRS	:= $(INCLUDE) $(INCLUDE)/glm
LIBDIRS		:= $(LIB)
FIXPATH = $(subst /,/,$1)
# Use a robust Windows delete command (cmd) ignoring missing files
RM			:= cmd /C del /Q /F
MD	:= mkdir
# Strip GUI subsystem flag if injected via env/CLI
override CXXFLAGS := $(filter-out -mwindows,$(CXXFLAGS))
override LFLAGS := $(filter-out -mwindows,$(LFLAGS))
# Force console subsystem to use main() (avoids WinMain link error)
override LFLAGS += -mconsole
RM_OUTPUT := $(RM) $(call FIXPATH,$(OUTPUT)/$(MAIN)) 2>nul || echo "Output file not found"
RM_OBJS := $(RM) /S $(call FIXPATH,src\*.o) 2>nul || echo "Object files not found"
else
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
LIBRARIES	:= -lglfw -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
else
LIBRARIES	:= -lglfw -lGL -ldl -lpthread
endif
MAIN	:= main
SOURCEDIRS	:= $(shell find $(SRC) -type d)
INCLUDEDIRS	:= $(shell find $(INCLUDE) -type d)
LIBDIRS		:= $(shell find $(LIB) -type d)
FIXPATH = $1
RM = rm -f
MD	:= mkdir -p
RM_OUTPUT := $(RM) $(OUTPUT)/$(MAIN) 2>/dev/null || echo "Output file not found"
RM_OBJS := find src -name '*.o' -type f -exec rm -f {} + >/dev/null 2>&1 || echo "Object files not found"
endif

# define any directories containing header files other than /usr/include
INCLUDES	:= $(patsubst %,-I%, $(INCLUDEDIRS:%/=%))

# define the C libs
LIBS		:= $(patsubst %,-L%, $(LIBDIRS:%/=%))

# define the C source files
SOURCES		:= $(wildcard $(patsubst %,%/*.cpp, $(SOURCEDIRS)))
SOURCES		:= $(filter-out $(SRC)/main/simple_test.cpp, $(SOURCES))

# Optional Dear ImGui integration (set USE_IMGUI=1 when invoking make if you actually have the sources)
# Expected layout (if enabled): include/imgui/ with official imgui*.cpp + backends/imgui_impl_glfw.cpp & imgui_impl_opengl3.cpp (you may need to adjust paths)
USE_IMGUI ?= 0
ifeq ($(USE_IMGUI),1)
IMGUI_DIR := include/imgui
# Basic core sources (adjust if you add more, e.g. imgui_tables.cpp, imgui_demo.cpp)
IMGUI_SOURCES := \
	$(IMGUI_DIR)/imgui.cpp \
	$(IMGUI_DIR)/imgui_draw.cpp \
	$(IMGUI_DIR)/imgui_widgets.cpp \
	$(IMGUI_DIR)/imgui_tables.cpp \
	$(IMGUI_DIR)/imgui_demo.cpp \
	$(IMGUI_DIR)/imgui_impl_glfw.cpp \
	$(IMGUI_DIR)/imgui_impl_opengl3.cpp
# Only append sources that actually exist to avoid build breaks if incomplete
EXISTING_IMGUI_SOURCES := $(wildcard $(IMGUI_SOURCES))
ifneq ($(EXISTING_IMGUI_SOURCES),)
	SOURCES += $(EXISTING_IMGUI_SOURCES)
else
	$(warning USE_IMGUI=1 but no ImGui source files found under $(IMGUI_DIR); skipping ImGui integration)
endif
endif

# define the C object files 
OBJECTS		:= $(SOURCES:.cpp=.o)

#
# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

OUTPUTMAIN	:= $(call FIXPATH,$(OUTPUT)/$(MAIN))

all: $(OUTPUT) $(MAIN)
	@echo Executing 'all' complete!

debug: BUILD_TYPE := Debug
debug: all

release: BUILD_TYPE := Release
release: all

$(OUTPUT):
	$(MD) $(OUTPUT)

$(MAIN): $(OBJECTS) 
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(OUTPUTMAIN) $(OBJECTS) $(LFLAGS) $(LIBS) $(LIBRARIES)

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file) 
# (see the gnu make manual section about automatic variables)
.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $<  -o $@

.PHONY: clean rebuild
clean:
	@echo Cleaning output and objects...
	$(RM_OUTPUT)
	$(RM_OBJS)
	@echo Cleanup complete!

rebuild: clean all
	@echo Rebuild complete!
# 此处./src/$(dir) 传递main函数 argv 的参数
run: all
	./$(OUTPUTMAIN) src/$(dir)/
	@echo Executing 'run: all' complete!
