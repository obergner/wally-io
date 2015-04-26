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
SOURCES         += $(wildcard src/*.cpp)
SOURCES         += $(wildcard src/io_wally/*.cpp)
SOURCES         += $(wildcard src/io_wally/protocol/*.cpp)

# Build dir for sources
MAINBUILD       := $(BUILD)/main

# Objects
MAINOBJS	    := $(patsubst src/%.cpp, $(MAINBUILD)/%.o, $(SOURCES))

# Subdirs in build directory need to reflect subdirs in src directory
MAINBUILDDIRS	:= $(sort $(dir $(MAINOBJS)))

# Main executable
MAIN_EXEC 	    := $(MAINBUILD)/mqtt-serverd

# Test compiler flags
TESTCFLAGS      = $(CFLAGS)
TESTCFLAGS      += -I ./test

# Test linker flags
TESTLFLAGS      = $(LFLAGS)

# Test sources
TESTSOURCES     += $(wildcard test/*.cpp)
TESTSOURCES     += $(wildcard test/io_wally/*.cpp)
TESTSOURCES     += $(wildcard test/io_wally/protocol/*.cpp)

# Build dir for tests
TESTBUILD       := $(BUILD)/test

# Test objects
TESTOBJS	    := $(patsubst test/%.cpp, $(TESTBUILD)/%.o, $(TESTSOURCES))

# Subdirs in build directory need to reflect subdirs in test directory
TESTBUILDDIRS	:= $(sort $(dir $(TESTOBJS)))

# Main executable
TEST_EXEC 	    := $(TESTBUILD)/all-tests

# What may be rebuilt
REBUILDABLES 	= $(MAIN_EXEC) $(MAINOBJS)

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
main				: $(MAIN_EXEC) 		| $(MAINBUILDDIRS)

$(MAINBUILDDIRS)	:
	@mkdir -p $@

$(MAIN_EXEC)		: $(MAINOBJS) 		| $(MAINBUILDDIRS)
	$(CC) $(LFLAGS) -o $@ $^

$(MAINBUILD)/%.o	: src/%.cpp		| $(MAINBUILDDIRS)
	$(CC) $(CFLAGS) -o $@ -c $<

# Test
.PHONY 				: test
test 				: $(TEST_EXEC) 		| $(TESTBUILDDIRS)
	@./$(TEST_EXEC) --success --durations yes

$(TESTBUILDDIRS)	:
	@mkdir -p $@

$(TEST_EXEC)		: $(TESTOBJS) 		| $(MAINBUILDDIRS)
	$(CC) $(LFLAGS) -o $@ $^

$(TESTBUILD)/%.o	: test/%.cpp		| $(TESTBUILDDIRS)
	$(CC) $(TESTCFLAGS) -o $@ -c $<

# Clean
.PHONY 				: clean
clean				:
	@rm -rf $(BUILD)

# Tools
.PHONY				: macroexpand
macroexpand			: $(SOURCES)
	$(CC) $(CFLAGS) -E $(SOURCES) | source-highlight --failsafe --src-lang=cc -f esc --style-file=esc.style 

.PHONY 				: check-all
check-all 			: $(SOURCES)
	clang-check $(SOURCES)

.PHONY 				: tags
tags 				: $(SOURCES) $(TESTSOURCES)
	@ctags -R -f ./.tags ./src ./test

###############################################################################
# Dependency rules: http://stackoverflow.com/questions/8025766/makefile-auto-dependency-generation
###############################################################################

-include 	$(MAINOBJS:.o=.d)
-include 	$(TESTOBJS:.o=.d)
