SOURCEDIR := $(abspath $(patsubst %/,%,$(dir $(abspath $(lastword \
	$(MAKEFILE_LIST))))))

CC ?= cc
CXX ?= c++
CFLAGS ?= -O2
CXXFLAGS ?= -O2
FLAGS := -std=c++17

INCLUDES := -I$(SOURCEDIR)/..
WARNINGS :=
LIBS :=  -ldl

NAME := bass
PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin

UNAME := $(shell uname -s)
ifeq ($(UNAME), Linux)
	LIBS += -Wl,--no-undefined
endif

ifeq ($(shell $(CXX) -v 2>&1 | grep -c "clang"),1)
	WARNINGS += -Wno-shift-count-overflow -Wno-parentheses -Wno-switch
endif

CXXSRCS := bass.cpp

OBJDIR := objs

# List of object files
OBJS := $(patsubst %,$(OBJDIR)/%,$(CXXSRCS:.cpp=.o))

# Compiler commands
COMPILE = $(strip $(1) $(CPPFLAGS) $(PIC) $(2) -c $< -o $@)
COMPILE_CXX = $(call COMPILE, $(CXX) $(CXXFLAGS), $(1))

# Info command
COMPILE_INFO = $(info $(subst $(SOURCEDIR)/,,$(1)))

# Core commands
BUILD_MAIN = $(call COMPILE_CXX, $(FLAGS) $(WARNINGS) $(INCLUDES))

.PHONY: all clean install install-strip uninstall

all: $(NAME)

$(OBJDIR)/%.o: $(SOURCEDIR)/%.cpp $(OBJDIR)/.tag
	$(call COMPILE_INFO, $(BUILD_MAIN))
	@$(BUILD_MAIN)

$(OBJDIR)/.tag:
	@mkdir -p -- $(OBJDIR)
	@touch $@

$(NAME): $(OBJS)
	$(CXX) $^ $(LDFLAGS) $(LIBS) $(SHARED) -o $@

clean:
	rm -rf $(OBJDIR)/ $(NAME)

install: all
	@mkdir -p $(DESTDIR)$(BINDIR)
	cp $(NAME) $(DESTDIR)$(BINDIR)

install-strip: install
	strip $(DESTDIR)$(BINDIR)/$(NAME)

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(NAME)
