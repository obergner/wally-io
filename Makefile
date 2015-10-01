###############################################################################
# Macros
###############################################################################

# GCC
CXX		        := g++
#CXX		     := clang++

# Standard compiler flags
CXXFLAGS	    += -std=c++11
CXXFLAGS        += -fdiagnostics-color=auto
CXXFLAGS        += -MMD # automatically generate dependency rules on each run
CXXFLAGS        += -I ./src
CXXFLAGS	    += -Werror
CXXFLAGS	    += -Wall
CXXFLAGS	    += -Wextra
CXXFLAGS	    += -Wcast-align
CXXFLAGS	    += -Wformat-nonliteral
CXXFLAGS	    += -Wformat=2
CXXFLAGS	    += -Winvalid-pch
CXXFLAGS	    += -Wmissing-declarations
CXXFLAGS	    += -Wmissing-format-attribute
CXXFLAGS	    += -Wmissing-include-dirs
CXXFLAGS	    += -Wredundant-decls
CXXFLAGS	    += -Wswitch-default
CXXFLAGS	    += -Wswitch-enum

CXXRELEASE_FLAGS	+= -O3 # -dNDEBUG
CXXDEBUG_FLAGS	+= -O0 -g
CXXDEBUG_FLAGS	+= -D_GLIBCXX_DEBUG
CXXDEBUG_FLAGS	+= -DBOOST_ASIO_ENABLE_HANDLER_TRACKING

# Extra compiler flags
CXXFLAGS 		+= $(shell pkg-config --cflags libsystemd-journal)

# Extra linker flags
LDLIBS		    += $(shell pkg-config --libs libsystemd-journal)
LDLIBS          += -lboost_system
LDLIBS          += -lboost_thread
LDLIBS          += -lboost_log
LDLIBS          += -lboost_log_setup
LDLIBS          += -lboost_program_options
LDLIBS          += -lpthread

# Top level build directory
# see: http://blog.kompiler.org/post/6/2011-09-18/Separate_build_and_source_directories_in_Makefiles/
BUILD 		    := build

# Sources
MSOURCES         += $(wildcard src/io_wally/*.cpp)
MSOURCES         += $(wildcard src/io_wally/protocol/*.cpp)
MSOURCES         += $(wildcard src/io_wally/codec/*.cpp)
MSOURCES         += $(wildcard src/io_wally/spi/*.cpp)
MSOURCES         += $(wildcard src/io_wally/impl/*.cpp)
MSOURCES         += $(wildcard src/io_wally/app/*.cpp)

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
TCXXFLAGS      	:= $(CXXFLAGS)
TCXXFLAGS      	+= -O0 -g
TCXXFLAGS      	+= -I ./test
TCXXFLAGS      	:= $(filter-out -Wswitch-default, $(TCXXFLAGS))
TCXXFLAGS      	:= $(filter-out -Wswitch-enum, $(TCXXFLAGS))

# Test linker flags
TLDLIBS      	= $(LDLIBS)

# Test sources
TSOURCES     	+= $(wildcard test/*.cpp)
TSOURCES     	+= $(wildcard test/io_wally/*.cpp)
TSOURCES     	+= $(wildcard test/io_wally/protocol/*.cpp)
TSOURCES     	+= $(wildcard test/io_wally/codec/*.cpp)
TSOURCES     	+= $(wildcard test/io_wally/impl/*.cpp)
TSOURCES     	+= $(wildcard test/io_wally/app/*.cpp)

TEXECSOURCE     := $(wildcard test/*.cpp)

# Build dir for tests
TBUILD       	:= $(BUILD)/test

# Test objects
TOBJS	    	:= $(patsubst test/%.cpp, $(TBUILD)/%.o, $(TSOURCES))

TEXECOBJ    	:= $(patsubst test/%.cpp, $(TBUILD)/%.o, $(TEXECSOURCE))

# Subdirs in build directory need to reflect subdirs in test directory
TBUILDDIRS		:= $(sort $(dir $(TOBJS)))

# Main test executable
TEXEC 	    	:= $(TBUILD)/all-tests

# Where to store scan-build's analysis results
SBUILD          := $(BUILD)/scan

# What may be rebuilt
REBUILDABLES 	= $(MEXEC) $(MOBJS) $(TEXEC) $(TOBJS) 

# All things doxygen
DOCDIR			:= $(BUILD)/doc

# Clang's compilation database needed for some of its tooling
COMPILATIONDB   := compile_commands.json

###############################################################################
# Rules
###############################################################################

# Main
.PHONY 				: release
release				: CXXFLAGS += $(CXXRELEASE_FLAGS)
release				: main

.PHONY 				: debug
debug				: CXXFLAGS += $(CXXDEBUG_FLAGS)
debug				: main

.PHONY 				: main
main				: $(MEXEC) 						| $(MBUILDDIRS)

$(MBUILDDIRS)		:
	@mkdir -p $@

$(MEXEC)			: $(MOBJS) $(MEXECOBJ) 			| $(MBUILDDIRS)
	$(CXX) $(LDLIBS) -o $@ $^

$(MBUILD)/%.o		: src/%.cpp						| $(MBUILDDIRS)
	$(CXX) $(CXXFLAGS) -o $@ -c $<

# Test
.PHONY 				: test
test 				: test-compile
	@./$(TEXEC) --success --durations yes

.PHONY 				: test-compile
test-compile 		: $(TEXEC)						| $(TBUILDDIRS)

$(TBUILDDIRS)		:
	@mkdir -p $@

$(TEXEC)			: $(MOBJS) $(TOBJS) $(TEXECOBJ)	| $(MBUILDDIRS)
	$(CXX) $(LDLIBS) -o $@ $^

$(TBUILD)/%.o		: test/%.cpp					| $(TBUILDDIRS)
	$(CXX) $(TCXXFLAGS) -o $@ -c $<

# Clean
.PHONY 				: clean
clean				:
	@rm -rf $(BUILD)

# Documentation
.PHONY 				: doc
doc 				: $(MSOURCES) $(MEXECSOURCE)
	@rm -rf $(DOCDIR)
	@doxygen ./.doxygen.cfg

# Tools
compilation-db 		: $(COMPILATIONDB)

$(COMPILATIONDB)    : $(MSOURCES) $(MEXECSOURCE) $(TSOURCES) $(TEXECSOURCE)
	@bear $(MAKE) clean main test-compile

.PHONY				: macroexpand
macroexpand			: $(MSOURCES) $(MEXECSOURCE)
	$(CXX) $(CXXFLAGS) -E $(MSOURCES) | source-highlight --failsafe --src-lang=cc -f esc --style-file=esc.style 

# Running clang-check
.PHONY 				: check-main
check-main 			: $(MSOURCES) $(MEXECSOURCE)
	@clang-check $(MSOURCES) $(MEXECSOURCE)

.PHONY 				: check-test
check-test 			: $(TSOURCES) $(TEXECSOURCE)
	@clang-check $(TSOURCES) $(TEXECSOURCE)

.PHONY 				: check
check	 			: check-main check-test

# Running scan-build
$(SBUILD)           :
	@mkdir -p $@

.PHONY 				: scan-main
scan-main 			: $(SBUILD)
	@scan-build -o $(SBUILD) -analyze-headers --status-bugs $(MAKE) clean release

.PHONY 				: modernize
modernize 			: $(MSOURCES) $(MEXECSOURCE) $(COMPILATIONDB)
	@clang-modernize -final-syntax-check -summary -format -style=file -include=src/ -p $(COMPILATIONDB)

.PHONY 				: format-main
format-main			: $(MSOURCES) $(MEXECSOURCE)
	@clang-format -i -style=file $(MSOURCES) $(MEXECSOURCE)

.PHONY 				: format-test
format-test			: $(TSOURCES) $(TEXECSOURCE)
	@clang-format -i -style=file $(TSOURCES) $(TEXECSOURCE)

.PHONY 				: format
format  			: format-main format-test

.PHONY 				: tags
tags 				: $(MSOURCES) $(MEXECSOURCE) $(TSOURCES) $(TEXECSOURCE)
	@ctags -R -f ./.tags ./src ./test

.PHONY 				: prepare-commit
prepare-commit 		: clean
prepare-commit 		: format
prepare-commit 		: main
prepare-commit 		: test
prepare-commit 		: check
prepare-commit 		: scan-main

###############################################################################
# Dependency rules: http://stackoverflow.com/questions/8025766/makefile-auto-dependency-generation
###############################################################################

-include 	$(MOBJS:.o=.d)
-include 	$(TOBJS:.o=.d)
