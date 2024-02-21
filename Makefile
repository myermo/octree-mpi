# Generic Makefile


# Project name (generate executable with this name)
TARGET = main

# Release name (name of the zip when executing make dist)
DIST_NAME = release

# Compiler
CC = g++

##########################
# Commands1
##########################
MKDIR_P = mkdir -p
rm       = rm -f

##########################
### PARAMETERS ###
##########################
SRCDIR = src
INCDIR = inc
OBJDIR = obj
BINDIR = .
LIBDIR = lib

##########################
### Include directories (add here you include directories) ###
##########################
LASLIB_INC = $(LIBDIR)/LAStools/LASlib/inc
LASZIP_INC = $(LIBDIR)/LAStools/LASzip/src
EIGEN_INC = /usr/include/eigen3
SHP_INC = $(LIBDIR)/shapelib-1.5.0
# BITPITDIR = $(LIBDIR)/bitpit/src $(LIBDIR)/bitpit/build/src
# BITPIT_INC = $(shell find -L $(BITPITDIR) -regex '.*\.hp*' -printf "%h\n" | sort -u)
# BITPIT_INC = /usr/local/include/bitpit
# BITPIT_INC = $(LIBDIR)/bitpit/build/src $(LIBDIR)/bitpit/src

##########################
### Automagically include all directories under SRCDIR or INCDIR where a .h[pp] exists. ###
##########################
INC_DIRS := $(shell find -L $(SRCDIR) $(INCDIR) -regex '.*\.hp*' -printf "%h\n" | sort -u)
INC_DIRS += $(LASLIB_INC) $(LASZIP_INC) $(EIGEN_INC) $(SHP_INC) # $(BITPIT_INC)
INC_FLAGS := $(foreach d, $(INC_DIRS), -I$d)

##########################
### Libraries (add here the paths where the used libraries are located) ###
##########################
LASLIB_LIB = $(LIBDIR)/LAStools/LASlib/lib
SHP_LIB = $(LIBDIR)/shapelib-1.5.0
# BITPIT_LIB = $(LIBDIR)/bitpit/build
# BITPIT_LIB = /usr/local/include/bitpit
LIBRARY_DIRS = $(LASLIB_LIB) $(SHP_LIB) # $(BITPIT_LIB)
LIB = $(foreach d, $(LIBRARY_DIRS), -L$d)

LIB_SEARCH_PATH := $(foreach d, $(LIBRARY_DIRS), -Wl,-R$d)

##########################
### Libraries to link against ###
##########################
LINKING = -lLASlib -larmadillo -fopenmp $(LIB_SEARCH_PATH)

##########################
### Compiler warnings (add here your custom flags) ###
##########################
WARNINGS = -w

SOURCES  := $(shell find $(SRCDIR) -type f -regex ".*\.cp*")
OBJECTS  := $(patsubst $(SRCDIR)%,$(OBJDIR)%,$(patsubst %.cpp,%.o,$(SOURCES)))
SUBDIRS  := $(shell find $(SRCDIR) -type d)
OBJ_SUBDIRS := $(patsubst $(SRCDIR)%,$(OBJDIR)%,$(SUBDIRS))

##########################
### Compilation Flags ###
##########################
CXXFLAGS = $(INC_FLAGS) $(LIB) $(LINKING) $(WARNINGS) -O2 -std=c++20

##########################
### Linking of the main program ###
##########################
$(BINDIR)/$(TARGET): $(OBJECTS)
	$(CC) $^ $(CXXFLAGS) -o $@
	@echo "Linking complete!"

##########################
### Compilation of source files ###
##########################
$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.cpp | $(OBJ_SUBDIRS)
	@$(CC) $(CXXFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"

$(OBJ_SUBDIRS):
	$(MKDIR_P) $@

.PHONY: clean
clean:
	@$(rm) $(OBJECTS)
	@echo "Cleanup complete!"

.PHONY: remove
remove: clean
	@$(rm) $(BINDIR)/$(TARGET)
	@echo "Executable removed!"

.PHONY: dist
dist:
	@echo "Creating release"
	@rm -f $(DIST_NAME).zip
	@mkdir -p $(DIST_NAME)
	@cp -r $(SRCDIR) $(INCDIR) $(LIBDIR) $(DIST_NAME)
	@zip -r $(DIST_NAME).zip $(DIST_NAME)
	@rm -rf $(DIST_NAME)
	@echo "Release created"

.PHONY: info
info:
	$(info $(INC_DIRS))
	$(info $(SOURCES))
