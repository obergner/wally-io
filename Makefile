###############################################################################
# Macros
###############################################################################

# GCC
CC		        := g++

# Standard compiler flags
CFLAGS		    += -Wall
CFLAGS		    += -std=c++11
CFLAGS          += -fdiagnostics-color=auto
CFLAGS          += -MMD # automatically generate dependency rules on each run
CFLAGS          += -I ./src

CRELEASE_FLAGS	+= -O3 # -dNDEBUG
CDEBUG_FLAGS	+= -O0 -g

# Extra compiler flags
CFLAGS 		    += $(shell pkg-config --cflags libsystemd-journal)

# Extra linker flags
LFLAGS		    += $(shell pkg-config --libs libsystemd-journal)
LFLAGS          += -lboost_system
LFLAGS          += -lboost_thread
LFLAGS          += -lpthread

# Top level build directory
# see: http://blog.kompiler.org/post/6/2011-09-18/Separate_build_and_source_directories_in_Makefiles/
BUILD 		    := build

# Sources
MSOURCES         += $(wildcard src/io_wally/*.cpp)
MSOURCES         += $(wildcard src/io_wally/protocol/*.cpp)
MSOURCES         += $(wildcard src/io_wally/protocol/parser/*.cpp)

MEXECSOURCE      := $(wildcard src/*.cpp)

# Build dir for sources
MBUILD       	:= $(BUILD)/main

# Objects
MOBJS	    	:= $(patsubst src/%.cpp, $(MBUILD)/%.o, $(MSOURCES))

MEXECOBJ    	:= $(patsubst src/%.cpp, $(MBUILD)/%.o, $(MEXECSOURCE))

# Subdirs in build directory need to reflect subdirs in src directory
MBUILDDIRS		:= $(sort $(dir $(MOBJS)))

# Main executable
MEXEC 	    	:= $(MBUILD)/mqtt-serverd

# Test compiler flags
TCFLAGS      	= $(CFLAGS)
TCFLAGS      	+= -O0 -g
TCFLAGS      	+= -I ./test

# Test linker flags
TLFLAGS      	= $(LFLAGS)

# Test sources
TSOURCES     	+= $(wildcard test/*.cpp)
TSOURCES     	+= $(wildcard test/io_wally/*.cpp)
TSOURCES     	+= $(wildcard test/io_wally/protocol/*.cpp)
TSOURCES     	+= $(wildcard test/io_wally/protocol/parser/*.cpp)

TEXECSOURCE      := $(wildcard test/*.cpp)

# Build dir for tests
TBUILD       	:= $(BUILD)/test

# Test objects
TOBJS	    	:= $(patsubst test/%.cpp, $(TBUILD)/%.o, $(TSOURCES))

TEXECOBJ    	:= $(patsubst test/%.cpp, $(TBUILD)/%.o, $(TEXECSOURCE))

# Subdirs in build directory need to reflect subdirs in test directory
TBUILDDIRS		:= $(sort $(dir $(TOBJS)))

# Main executable
TEXEC 	    	:= $(TBUILD)/all-tests

# What may be rebuilt
REBUILDABLES 	= $(MEXEC) $(MOBJS) $(TEXEC) $(TOBJS) 

###############################################################################
# Rules
###############################################################################

# Main
.PHONY 				: release
release				: CFLAGS += $(CRELEASE_FLAGS)
release				: main

.PHONY 				: debug
debug				: CFLAGS += $(CDEBUG_FLAGS)
debug				: main

.PHONY 				: main
main				: $(MEXEC) 						| $(MBUILDDIRS)

$(MBUILDDIRS)		:
	@mkdir -p $@

$(MEXEC)			: $(MOBJS) $(MEXECOBJ) 			| $(MBUILDDIRS)
	$(CC) $(LFLAGS) -o $@ $^

$(MBUILD)/%.o		: src/%.cpp						| $(MBUILDDIRS)
	$(CC) $(CFLAGS) -o $@ -c $<

# Test
.PHONY 				: test
test 				: $(TEXEC) 						| $(TBUILDDIRS)
	@./$(TEXEC) --success --durations yes

$(TBUILDDIRS)		:
	@mkdir -p $@

$(TEXEC)			: $(MOBJS) $(TOBJS) $(TEXECOBJ)	| $(MBUILDDIRS)
	$(CC) $(LFLAGS) -o $@ $^

$(TBUILD)/%.o		: test/%.cpp					| $(TBUILDDIRS)
	$(CC) $(TCFLAGS) -o $@ -c $<

# Clean
.PHONY 				: clean
clean				:
	@rm -rf $(BUILD)

# Tools
.PHONY				: macroexpand
macroexpand			: $(MSOURCES) $(MEXECSOURCE)
	$(CC) $(CFLAGS) -E $(MSOURCES) | source-highlight --failsafe --src-lang=cc -f esc --style-file=esc.style 

.PHONY 				: check-main
check-main 			: $(MSOURCES) $(MEXECSOURCE)
	clang-check $(MSOURCES) $(MEXECSOURCE)

.PHONY 				: check-test
check-test 			: $(TSOURCES) $(TEXECSOURCE)
	clang-check $(TSOURCES) $(TEXECSOURCE)

.PHONY 				: tags
tags 				: $(MSOURCES) $(MEXECSOURCE) $(TSOURCES) $(TEXECSOURCE)
	@ctags -R -f ./.tags ./src ./test

###############################################################################
# Dependency rules: http://stackoverflow.com/questions/8025766/makefile-auto-dependency-generation
###############################################################################

-include 	$(MOBJS:.o=.d)
-include 	$(TOBJS:.o=.d)
