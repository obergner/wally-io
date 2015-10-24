#######################################################################################################################
# Macros
#######################################################################################################################

# --------------------------------------------------------------------------------------------------------------------- 
# Common definitions
# --------------------------------------------------------------------------------------------------------------------- 

# Top level build directory
# see: http://blog.kompiler.org/post/6/2011-09-18/Separate_build_and_source_directories_in_Makefiles/
BUILD           := build

# --------------------------------------------------------------------------------------------------------------------- 
# Target executable source
# --------------------------------------------------------------------------------------------------------------------- 

# SRCS
MSRCS           := $(wildcard src/io_wally/*.cpp)
MSRCS           += $(wildcard src/io_wally/protocol/*.cpp)
MSRCS           += $(wildcard src/io_wally/codec/*.cpp)
MSRCS           += $(wildcard src/io_wally/spi/*.cpp)
MSRCS           += $(wildcard src/io_wally/impl/*.cpp)
MSRCS           += $(wildcard src/io_wally/dispatch/*.cpp)
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

# --------------------------------------------------------------------------------------------------------------------- 
# Unit tests
# --------------------------------------------------------------------------------------------------------------------- 

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

# --------------------------------------------------------------------------------------------------------------------- 
# Integration tests
# --------------------------------------------------------------------------------------------------------------------- 

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

# --------------------------------------------------------------------------------------------------------------------- 
# Snippets: try stuff, analyse bugs etc.
# --------------------------------------------------------------------------------------------------------------------- 

# Snippets SRCS
SNEXECSOURCE    := itest/snippets_main.cpp

SNEXECOBJ       := $(patsubst itest/%.cpp, $(ITBUILD)/%.o, $(SNEXECSOURCE))

# Main integrationtest executable
SNEXEC          := $(ITBUILD)/snippets

# --------------------------------------------------------------------------------------------------------------------- 
# Third party libraries (included in this source tree): common
# --------------------------------------------------------------------------------------------------------------------- 

# Third-party libraries included in this source tree
THIRD_PARTY_LIBS := ./libs

# Build dir for third-party libraries
LIBS_BUILD      := $(BUILD)/libs

# --------------------------------------------------------------------------------------------------------------------- 
# Third party libraries (included in this source tree): Boost Asio Queue Extension by Hans Ewetz
# --------------------------------------------------------------------------------------------------------------------- 

# Boost ASIO Queue Extension by Hans Ewetz
BOOST_ASIO_QE   := $(THIRD_PARTY_LIBS)/boost-asio-queue-extension

# --------------------------------------------------------------------------------------------------------------------- 
# Third party libraries (included in this source tree): C++11-friendly header only dbus lib by Ubuntu
# --------------------------------------------------------------------------------------------------------------------- 

# C++11 DBus library by Ubuntu
DBUS_CPP        := $(THIRD_PARTY_LIBS)/dbus-cpp
DBUS_CPP_INC    := $(DBUS_CPP)/include
DBUS_CPP_SRC    := $(DBUS_CPP)/src

# --------------------------------------------------------------------------------------------------------------------- 
# Tooling
# --------------------------------------------------------------------------------------------------------------------- 

# Where to store scan-build's analysis results
SBUILD          := $(BUILD)/scan

# All things doxygen
DOCDIR          := $(BUILD)/doc

# Clang's compilation database needed for some of its tooling
COMPILATIONDB   := compile_commands.json

# --------------------------------------------------------------------------------------------------------------------- 
# Miscellaneous
# --------------------------------------------------------------------------------------------------------------------- 

# What may be rebuilt
REBUILDABLES    := $(MEXEC) $(MOBJS) $(TEXEC) $(TOBJS) $(ITEXEC) $(ITOBJS)

# --------------------------------------------------------------------------------------------------------------------- 
# Compiler configuration: common
# --------------------------------------------------------------------------------------------------------------------- 

# GCC
CXX             := g++
#CXX             := clang++
CC              := gcc

# --------------------------------------------------------------------------------------------------------------------- 
# Compiler configuration: main executable
# --------------------------------------------------------------------------------------------------------------------- 

# Standard compiler flags
CXXFLAGS        := -std=c++14
CXXFLAGS        += -fdiagnostics-color=auto
CXXFLAGS        += -MMD # automatically generate dependency rules on each run
CXXFLAGS        += -I ./src
CXXFLAGS        += -I $(BOOST_ASIO_QE)
CXXFLAGS        += -I $(DBUS_CPP_INC)
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

CXXRELEASE_FLAGS := -O3 # -dNDEBUG
CXXDEBUG_FLAGS  := -O0 -g
CXXDEBUG_FLAGS  += -D_GLIBCXX_DEBUG
CXXDEBUG_FLAGS  += -DBOOST_ASIO_ENABLE_HANDLER_TRACKING

# Extra linker flags
LDLIBS          := -lboost_system
LDLIBS          += -lboost_thread
LDLIBS          += -lboost_log
LDLIBS          += -lboost_log_setup
LDLIBS          += -lboost_program_options
LDLIBS          += -lpthread

# --------------------------------------------------------------------------------------------------------------------- 
# Compiler configuration: unit tests
# --------------------------------------------------------------------------------------------------------------------- 

# Test compiler flags
TCXXFLAGS       := $(CXXFLAGS)
TCXXFLAGS       += -O0 -g
TCXXFLAGS       += -I ./test
TCXXFLAGS       := $(filter-out -Wswitch-default, $(TCXXFLAGS))
TCXXFLAGS       := $(filter-out -Wswitch-enum, $(TCXXFLAGS))

# Test linker flags
TLDLIBS         := $(LDLIBS)

# --------------------------------------------------------------------------------------------------------------------- 
# Compiler configuration: integration tests
# --------------------------------------------------------------------------------------------------------------------- 

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

# --------------------------------------------------------------------------------------------------------------------- 
# Compiler configuration: dbus-cpp
# --------------------------------------------------------------------------------------------------------------------- 

# Standard compiler flags
DBCXXFLAGS      := -std=c++11
DBCXXFLAGS      += -fdiagnostics-color=auto
DBCXXFLAGS      += -MMD # automatically generate dependency rules on each run
DBCXXFLAGS      += -I ./src
DBCXXFLAGS      += -I $(BOOST_ASIO_QE)
DBCXXFLAGS      += -I $(DBUS_CPP_INC)
DBCXXFLAGS      += -Werror
DBCXXFLAGS      += -Wall
DBCXXFLAGS      += -Wextra
DBCXXFLAGS      += -Wcast-align
DBCXXFLAGS      += -Wformat-nonliteral
DBCXXFLAGS      += -Wformat=2
DBCXXFLAGS      += -Winvalid-pch
DBCXXFLAGS      += -Wmissing-declarations
DBCXXFLAGS      += -Wmissing-format-attribute
DBCXXFLAGS      += -Wmissing-include-dirs
DBCXXFLAGS      += -Wredundant-decls
DBCXXFLAGS      += -Wswitch-default
DBCXXFLAGS      += -Wswitch-enum

# Standard preprocessor flags
DBCPPFLAGS      := -DBOOST_ALL_DYN_LINK
# Needed for clang:
DBCPPFLAGS      += -DBOOST_ASIO_HAS_STD_CHRONO 

# Extra linker flags
DBLDLIBS        := -lboost_system
DBLDLIBS        += -lboost_thread
DBLDLIBS        += -lboost_program_options
DBLDLIBS        += -lpthread


#######################################################################################################################
# Rules
#######################################################################################################################

# --------------------------------------------------------------------------------------------------------------------- 
# Build main executable
# --------------------------------------------------------------------------------------------------------------------- 

.PHONY              : release
release             : CXXFLAGS += $(CXXRELEASE_FLAGS)
release             : main

.PHONY              : debug
debug               : CXXFLAGS += $(CXXDEBUG_FLAGS)
debug               : main

main                : $(MEXEC)                         | $(MBUILDDIRS)

$(MBUILDDIRS)       :
	@mkdir -p $@

$(MEXEC)            : $(MOBJS) $(MEXECOBJ)             | $(MBUILDDIRS)
	$(CXX) $(LDLIBS) -o $@ $^

$(MBUILD)/%.o       : src/%.cpp                        | $(MBUILDDIRS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<

# --------------------------------------------------------------------------------------------------------------------- 
# Build/run unit tests
# --------------------------------------------------------------------------------------------------------------------- 

.PHONY              : test
test                : test-compile
	@./$(TEXEC)

.PHONY              : test-success
test-success        : test-compile
	@./$(TEXEC) --success --durations yes

test-compile        : $(TEXEC)                         | $(TBUILDDIRS)

$(TBUILDDIRS)       :
	@mkdir -p $@

$(TEXEC)            : $(MOBJS) $(TOBJS) $(TEXECOBJ)    | $(TBUILDDIRS)
	$(CXX) $(TLDLIBS) -o $@ $^

$(TBUILD)/%.o       : test/%.cpp                       | $(TBUILDDIRS)
	$(CXX) $(TCXXFLAGS) -o $@ -c $<

# --------------------------------------------------------------------------------------------------------------------- 
# Build Paho MQTT client to support integration tests
# --------------------------------------------------------------------------------------------------------------------- 

$(PAHOPBUILD)       :
	@mkdir -p $@

$(PAHOPBUILD)/%.o   : support/paho/packet/%.c          | $(PAHOPBUILD)
	$(CC) -o $@ -c $<

$(PAHOCBUILD)       :
	@mkdir -p $@

$(PAHOCBUILD)/%.o   : support/paho/client/%.cpp        | $(PAHOCBUILD)
	$(CXX) -o $@ -c $<

# --------------------------------------------------------------------------------------------------------------------- 
# Build/run integration tests
# --------------------------------------------------------------------------------------------------------------------- 

.PHONY              : itest
itest               : itest-compile
	@./$(ITEXEC) --success --durations yes

itest-compile       : $(ITEXEC)                        | $(ITBUILDDIRS)

$(ITBUILDDIRS)      :
	@mkdir -p $@

$(ITEXEC)           : $(MOBJS) $(ITOBJS) $(ITEXECOBJ) $(PAHOOBJS)    | $(ITBUILDDIRS)
	$(CXX) $(ITLDLIBS) -o $@ $^

$(ITBUILD)/%.o      : itest/%.cpp                      | $(ITBUILDDIRS)
	$(CXX) $(CPPFLAGS) $(ITCXXFLAGS) -o $@ -c $<

# --------------------------------------------------------------------------------------------------------------------- 
# Build snippets: try stuff, analyse bugs etc
# --------------------------------------------------------------------------------------------------------------------- 

.PHONY              : snippets 
snippets            : snippets-compile
	@./$(SNEXEC)

.PHONY              : snippets-compile
snippets-compile    : $(SNEXEC)                        | $(ITBUILDDIRS)

$(SNEXEC)           : $(MOBJS) $(ITOBJS) $(SNEXECOBJ)  | $(ITBUILDDIRS)
	$(CXX) $(ITLDLIBS) -o $@ $^

# --------------------------------------------------------------------------------------------------------------------- 
# Clean up the mess
# --------------------------------------------------------------------------------------------------------------------- 

.PHONY              : clean
clean               :
	@rm -rf $(BUILD)

# --------------------------------------------------------------------------------------------------------------------- 
# Generate/publis documentation
# --------------------------------------------------------------------------------------------------------------------- 

.PHONY              : doc
doc                 : $(MSRCS) $(MEXECSOURCE)
	@rm -rf $(DOCDIR)
	@doxygen ./.doxygen.cfg

.PHONY              : doc-publish
doc-publish         : doc
	@pushd $(DOCDIR); python -mSimpleHTTPServer 8000; popd

# --------------------------------------------------------------------------------------------------------------------- 
# Tooling
# --------------------------------------------------------------------------------------------------------------------- 

compilation-db      : $(COMPILATIONDB)

$(COMPILATIONDB)    : $(MSRCS) $(MEXECSOURCE) $(TSRCS) $(TEXECSOURCE) $(ITSRCS) $(ITEXECSOURCE)
	@bear $(MAKE) clean main test-compile itest-compile

.PHONY              : macroexpand
macroexpand         : $(MSRCS) $(MEXECSOURCE)
	$(CXX) $(CXXFLAGS) -E $(MSRCS) | source-highlight --failsafe --src-lang=cc -f esc --style-file=esc.style 

# Running clang-check
.PHONY              : check-main
check-main          : compilation-db
check-main          : $(MSRCS) $(MEXECSOURCE)
	@clang-check $(MSRCS) $(MEXECSOURCE)

.PHONY              : check-test
check-test          : compilation-db
check-test          : $(TSRCS) $(TEXECSOURCE)
	@clang-check $(TSRCS) $(TEXECSOURCE)

.PHONY              : check-itest
check-itest         : compilation-db
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

# --------------------------------------------------------------------------------------------------------------------- 
# Prepare a commit: run (almost) everything
# --------------------------------------------------------------------------------------------------------------------- 

.PHONY              : prepare-commit
prepare-commit      : clean
prepare-commit      : format
prepare-commit      : main
prepare-commit      : test 
#prepare-commit      : itest # Sadly, we have to deactivate this for now: integration tests crash when stopping app
prepare-commit      : check
prepare-commit      : scan-main

# --------------------------------------------------------------------------------------------------------------------- 
# Helpers
# --------------------------------------------------------------------------------------------------------------------- 

# Run server with some convenient default settings
.PHONY              : run-server
run-server          : $(MEXEC)
	@$(MEXEC) --log-file .testlog --log-file-level trace --log-console --log-console-level trace --conn-timeout 1000000

#######################################################################################################################
# Dependency rules: http://stackoverflow.com/questions/8025766/makefile-auto-dependency-generation
#######################################################################################################################

-include            $(MOBJS:.o=.d)
-include            $(MEXECOBJ:.o=.d)
-include            $(TOBJS:.o=.d)
-include            $(TEXECOBJ:.o=.d)
-include            $(ITOBJS:.o=.d)
-include            $(ITEXECOBJ:.o=.d)
