###############################################################################
# Macros
###############################################################################

# GCC
CXX             := g++
#CXX             := clang++
CC              := gcc

# Standard compiler flags
CXXFLAGS        := -std=c++14
CXXFLAGS        += -fdiagnostics-color=auto
CXXFLAGS        += -MMD # automatically generate dependency rules on each run
CXXFLAGS        += -I ./src
CXXFLAGS        += -I ./libs/boost-asio-queue-extension
CXXFLAGS        += -Werror
CXXFLAGS        += -Wall
CXXFLAGS        += -Wextra
CXXFLAGS        += -Wcast-align
CXXFLAGS        += -Wformat-nonliteral
CXXFLAGS        += -Wformat=2
CXXFLAGS        += -Winvalid-pch
CXXFLAGS        += -Wmissing-declarations
CXXFLAGS        += -Wmissing-format-attribute
CXXFLAGS        += -Wmissing-include-dirs
CXXFLAGS        += -Wredundant-decls
CXXFLAGS        += -Wswitch-default
CXXFLAGS        += -Wswitch-enum

# Standard preprocessor flags
CPPFLAGS        := -DBOOST_ALL_DYN_LINK
# Needed for clang:
# http://stackoverflow.com/questions/27552028/who-is-failing-boost-clang-or-gcc-issue-with-stdchrono-used-with-boostas
CPPFLAGS        += -DBOOST_ASIO_HAS_STD_CHRONO 

CXXRELEASE_FLAGS += -O3 # -dNDEBUG
CXXDEBUG_FLAGS  += -O0 -g
CXXDEBUG_FLAGS  += -D_GLIBCXX_DEBUG
CXXDEBUG_FLAGS  += -DBOOST_ASIO_ENABLE_HANDLER_TRACKING

# Extra linker flags
LDLIBS          := -lboost_system
LDLIBS          += -lboost_thread
LDLIBS          += -lboost_log
LDLIBS          += -lboost_log_setup
LDLIBS          += -lboost_program_options
LDLIBS          += -lpthread

# Top level build directory
# see: http://blog.kompiler.org/post/6/2011-09-18/Separate_build_and_source_directories_in_Makefiles/
BUILD           := build

# SRCS
MSRCS           := $(wildcard src/io_wally/*.cpp)
MSRCS           += $(wildcard src/io_wally/protocol/*.cpp)
MSRCS           += $(wildcard src/io_wally/codec/*.cpp)
MSRCS           += $(wildcard src/io_wally/spi/*.cpp)
MSRCS           += $(wildcard src/io_wally/impl/*.cpp)
MSRCS           += $(wildcard src/io_wally/app/*.cpp)

MEXECSOURCE     := $(wildcard src/*.cpp)

# Build dir for SRCS
MBUILD          := $(BUILD)/main

# Objects
MOBJS           := $(patsubst src/%.cpp, $(MBUILD)/%.o, $(MSRCS))

MEXECOBJ        := $(patsubst src/%.cpp, $(MBUILD)/%.o, $(MEXECSOURCE))

# Subdirs in build directory need to reflect subdirs in src directory
MBUILDDIRS      := $(sort $(dir $(MOBJS)))

# Main executable
MEXEC           := $(MBUILD)/mqtt-serverd

# Test compiler flags
TCXXFLAGS       := $(CXXFLAGS)
TCXXFLAGS       += -O0 -g
TCXXFLAGS       += -I ./test
TCXXFLAGS       := $(filter-out -Wswitch-default, $(TCXXFLAGS))
TCXXFLAGS       := $(filter-out -Wswitch-enum, $(TCXXFLAGS))

# Test linker flags
TLDLIBS         := $(LDLIBS)

# Test SRCS
TSRCS           += $(wildcard test/io_wally/*.cpp)
TSRCS           += $(wildcard test/io_wally/protocol/*.cpp)
TSRCS           += $(wildcard test/io_wally/codec/*.cpp)
TSRCS           += $(wildcard test/io_wally/impl/*.cpp)
TSRCS           += $(wildcard test/io_wally/app/*.cpp)

TEXECSOURCE     := $(wildcard test/*.cpp)

# Build dir for tests
TBUILD          := $(BUILD)/test

# Test objects
TOBJS           := $(patsubst test/%.cpp, $(TBUILD)/%.o, $(TSRCS))

TEXECOBJ        := $(patsubst test/%.cpp, $(TBUILD)/%.o, $(TEXECSOURCE))

# Subdirs in build directory need to reflect subdirs in test directory
TBUILDDIRS      := $(sort $(dir $(TOBJS)))

# Main test executable
TEXEC           := $(TBUILD)/all-tests

# Support libraries
SUPPORTBUILD    := $(BUILD)/support

# Support: Paho MQTT client
PAHOPINC        := support/paho/packet
PAHOCINC        := support/paho/client

PAHOPSRCS       := $(wildcard support/paho/packet/*.c)
PAHOCSRCS       := $(wildcard support/paho/client/*.cpp)

PAHOPBUILD      := $(SUPPORTBUILD)/paho/packet
PAHOCBUILD      := $(SUPPORTBUILD)/paho/client

PAHOOBJS        := $(patsubst support/paho/packet/%.c, $(PAHOPBUILD)/%.o, $(PAHOPSRCS))
PAHOOBJS        += $(patsubst support/paho/client/%.cpp, $(PAHOCBUILD)/%.o, $(PAHOCSRCS))

# Integrationtest compiler flags
ITCXXFLAGS      := $(CXXFLAGS)
ITCXXFLAGS      += -O0 -g
ITCXXFLAGS      += -I ./itest
ITCXXFLAGS      += -I $(PAHOPINC)
ITCXXFLAGS      += -I $(PAHOCINC)
ITCXXFLAGS      := $(filter-out -Wswitch-default, $(ITCXXFLAGS))
ITCXXFLAGS      := $(filter-out -Wswitch-enum, $(ITCXXFLAGS))

# Integrationtest linker flags
ITLDLIBS        := $(LDLIBS)

# Integrationtest SRCS
ITSRCS          := $(wildcard itest/framework/*.cpp)
ITSRCS          += $(wildcard itest/io_wally/*.cpp)

ITEXECSOURCE    := itest/itests_main.cpp

# Build dir for integrationtests
ITBUILD         := $(BUILD)/itest

# Intgegrationtest objects
ITOBJS          := $(patsubst itest/%.cpp, $(ITBUILD)/%.o, $(ITSRCS))

ITEXECOBJ       := $(patsubst itest/%.cpp, $(ITBUILD)/%.o, $(ITEXECSOURCE))

# Subdirs in build directory need to reflect subdirs in integrationtest directory
ITBUILDDIRS     := $(sort $(dir $(ITOBJS)))

# Main integrationtest executable
ITEXEC          := $(ITBUILD)/all-integrationtests

# Where to store scan-build's analysis results
SBUILD          := $(BUILD)/scan

# What may be rebuilt
REBUILDABLES    := $(MEXEC) $(MOBJS) $(TEXEC) $(TOBJS) $(ITEXEC) $(ITOBJS)

# All things doxygen
DOCDIR          := $(BUILD)/doc

# Clang's compilation database needed for some of its tooling
COMPILATIONDB   := compile_commands.json

# Snippets SRCS
SNEXECSOURCE    := itest/snippets_main.cpp

SNEXECOBJ       := $(patsubst itest/%.cpp, $(ITBUILD)/%.o, $(SNEXECSOURCE))

# Main integrationtest executable
SNEXEC          := $(ITBUILD)/snippets

###############################################################################
# Rules
###############################################################################

# Main
.PHONY              : release
release             : CXXFLAGS += $(CXXRELEASE_FLAGS)
release             : main

.PHONY              : debug
debug               : CXXFLAGS += $(CXXDEBUG_FLAGS)
debug               : main

.PHONY              : main
main                : $(MEXEC)                         | $(MBUILDDIRS)

$(MBUILDDIRS)       :
	@mkdir -p $@

$(MEXEC)            : $(MOBJS) $(MEXECOBJ)             | $(MBUILDDIRS)
	$(CXX) $(LDLIBS) -o $@ $^

$(MBUILD)/%.o       : src/%.cpp                        | $(MBUILDDIRS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<

# Test
.PHONY              : test
test                : test-compile
	@./$(TEXEC)

.PHONY              : test-success
test-success        : test-compile
	@./$(TEXEC) --success --durations yes

.PHONY              : test-compile
test-compile        : $(TEXEC)                         | $(TBUILDDIRS)

$(TBUILDDIRS)       :
	@mkdir -p $@

$(TEXEC)            : $(MOBJS) $(TOBJS) $(TEXECOBJ)    | $(TBUILDDIRS)
	$(CXX) $(TLDLIBS) -o $@ $^

$(TBUILD)/%.o       : test/%.cpp                       | $(TBUILDDIRS)
	$(CXX) $(TCXXFLAGS) -o $@ -c $<

# Support: Paho MQTT client
$(PAHOPBUILD)       :
	@mkdir -p $@

$(PAHOPBUILD)/%.o   : support/paho/packet/%.c          | $(PAHOPBUILD)
	$(CC) -o $@ -c $<

$(PAHOCBUILD)       :
	@mkdir -p $@

$(PAHOCBUILD)/%.o   : support/paho/client/%.cpp        | $(PAHOCBUILD)
	$(CXX) -o $@ -c $<

# Integrationtests
.PHONY              : itest
itest               : itest-compile
	@./$(ITEXEC) --success --durations yes

.PHONY              : itest-compile
itest-compile       : $(ITEXEC)                        | $(ITBUILDDIRS)

$(ITBUILDDIRS)      :
	@mkdir -p $@

$(ITEXEC)           : $(MOBJS) $(ITOBJS) $(ITEXECOBJ) $(PAHOOBJS)    | $(ITBUILDDIRS)
	$(CXX) $(ITLDLIBS) -o $@ $^

$(ITBUILD)/%.o      : itest/%.cpp                      | $(ITBUILDDIRS)
	$(CXX) $(CPPFLAGS) $(ITCXXFLAGS) -o $@ -c $<

# Snippets
.PHONY              : snippets 
snippets            : snippets-compile
	@./$(SNEXEC)

.PHONY              : snippets-compile
snippets-compile    : $(SNEXEC)                        | $(ITBUILDDIRS)

$(SNEXEC)           : $(MOBJS) $(SNEXECOBJ)            | $(ITBUILDDIRS)
	$(CXX) $(ITLDLIBS) -o $@ $^

# Clean
.PHONY              : clean
clean               :
	@rm -rf $(BUILD)

# Documentation
.PHONY              : doc
doc                 : $(MSRCS) $(MEXECSOURCE)
	@rm -rf $(DOCDIR)
	@doxygen ./.doxygen.cfg

.PHONY              : doc-publish
doc-publish         : doc
	@pushd $(DOCDIR); python -mSimpleHTTPServer 8000; popd

# Tools
compilation-db      : $(COMPILATIONDB)

$(COMPILATIONDB)    : $(MSRCS) $(MEXECSOURCE) $(TSRCS) $(TEXECSOURCE) $(ITSRCS) $(ITEXECSOURCE)
	@bear $(MAKE) clean main test-compile itest-compile

.PHONY              : macroexpand
macroexpand         : $(MSRCS) $(MEXECSOURCE)
	$(CXX) $(CXXFLAGS) -E $(MSRCS) | source-highlight --failsafe --src-lang=cc -f esc --style-file=esc.style 

# Running clang-check
.PHONY              : check-main
check-main          : $(MSRCS) $(MEXECSOURCE)
	@clang-check $(MSRCS) $(MEXECSOURCE)

.PHONY              : check-test
check-test          : $(TSRCS) $(TEXECSOURCE)
	@clang-check $(TSRCS) $(TEXECSOURCE)

.PHONY              : check-itest
check-itest         : $(ITSRCS) $(ITEXECSOURCE)
	@clang-check $(ITSRCS) $(ITEXECSOURCE)

.PHONY              : check
check               : check-main check-test 

# Running scan-build
$(SBUILD)           :
	@mkdir -p $@

.PHONY              : scan-main
scan-main           : $(SBUILD)
	@scan-build -o $(SBUILD) -analyze-headers --status-bugs $(MAKE) clean release

.PHONY              : modernize
modernize           : $(MSRCS) $(MEXECSOURCE) $(COMPILATIONDB)
	@clang-modernize -final-syntax-check -summary -format -style=file -include=src/ -p $(COMPILATIONDB)

.PHONY              : format-main
format-main         : $(MSRCS) $(MEXECSOURCE)
	@clang-format -i -style=file $(MSRCS) $(MEXECSOURCE)

.PHONY              : format-test
format-test         : $(TSRCS) $(TEXECSOURCE)
	@clang-format -i -style=file $(TSRCS) $(TEXECSOURCE)

.PHONY              : format-itest
format-itest        : $(ITSRCS) $(ITEXECSOURCE)
	@clang-format -i -style=file $(ITSRCS) $(ITEXECSOURCE)

.PHONY              : format
format              : format-main format-test format-itest

.PHONY              : tags
tags                : $(MSRCS) $(MEXECSOURCE) $(TSRCS) $(TEXECSOURCE) $(ITSRCS) $(ITEXECSOURCE)
	@ctags -R -f ./.tags ./src ./test ./itest

.PHONY              : prepare-commit
prepare-commit      : clean
prepare-commit      : format
prepare-commit      : main
prepare-commit      : test
prepare-commit      : itest
prepare-commit      : check
prepare-commit      : scan-main

# Run server with some convenient default settings
.PHONY              : run-server
run-server          : $(MEXEC)
	$(MEXEC) --log-file .testlog --log-file-level trace --log-console --log-console-level trace --conn-timeout 1000000

###############################################################################
# Dependency rules: http://stackoverflow.com/questions/8025766/makefile-auto-dependency-generation
###############################################################################

-include            $(MOBJS:.o=.d)
-include            $(MEXECOBJ:.o=.d)
-include            $(TOBJS:.o=.d)
-include            $(TEXECOBJ:.o=.d)
-include            $(ITOBJS:.o=.d)
-include            $(ITEXECOBJ:.o=.d)
