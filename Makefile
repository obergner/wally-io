#######################################################################################################################
# Macros
#######################################################################################################################

# --------------------------------------------------------------------------------------------------------------------- 
# INCLUDES
# --------------------------------------------------------------------------------------------------------------------- 

include ./libs/libs.mk

# --------------------------------------------------------------------------------------------------------------------- 
# Common definitions
# --------------------------------------------------------------------------------------------------------------------- 

# Top level build directory
# see: http://blog.kompiler.org/post/6/2011-09-18/Separate_build_and_source_directories_in_Makefiles/
BUILD                    := target

# --------------------------------------------------------------------------------------------------------------------- 
# Main executable: sources
# --------------------------------------------------------------------------------------------------------------------- 

#
# SOURCES
#
SRC_DIR_M                := src/main/cpp

SRCS_M                   := $(wildcard $(SRC_DIR_M)/io_wally/*.cpp)
SRCS_M                   += $(wildcard $(SRC_DIR_M)/io_wally/protocol/*.cpp)
SRCS_M                   += $(wildcard $(SRC_DIR_M)/io_wally/codec/*.cpp)
SRCS_M                   += $(wildcard $(SRC_DIR_M)/io_wally/spi/*.cpp)
SRCS_M                   += $(wildcard $(SRC_DIR_M)/io_wally/impl/*.cpp)
SRCS_M                   += $(wildcard $(SRC_DIR_M)/io_wally/dispatch/*.cpp)
SRCS_M                   += $(wildcard $(SRC_DIR_M)/io_wally/app/*.cpp)

EXECSOURCE_M             := $(wildcard $(SRC_DIR_M)/*.cpp)

# ***********************************************************************
# OBJECTS: main
# *********************************************************************** 

# Build dir for SRCS
BUILD_M                  := $(BUILD)/main

# Objects
OBJS_M                   := $(patsubst $(SRC_DIR_M)/%.cpp, $(BUILD_M)/%.o, $(SRCS_M))

EXECOBJ_M                := $(patsubst $(SRC_DIR_M)/%.cpp, $(BUILD_M)/%.o, $(EXECSOURCE_M))

# Subdirs in build directory need to reflect subdirs in src directory
BUILDDIRS_M              := $(sort $(dir $(OBJS_M)))

# Main executable
EXEC_M                   := $(BUILD_M)/wally-iod

# ***********************************************************************
# OBJECTS: release
# ***********************************************************************

# Build dir for SRCS
BUILD_M_RELEASE          := $(BUILD)/release

# Objects
OBJS_M_RELEASE           := $(patsubst $(SRC_DIR_M)/%.cpp, $(BUILD_M_RELEASE)/%.o, $(SRCS_M))

EXECOBJ_M_RELEASE        := $(patsubst $(SRC_DIR_M)/%.cpp, $(BUILD_M_RELEASE)/%.o, $(EXECSOURCE_M))

# Subdirs in build directory need to reflect subdirs in src directory
BUILDDIRS_M_RELEASE      := $(sort $(dir $(OBJS_M_RELEASE)))

# Main executable
EXEC_M_RELEASE           := $(BUILD_M_RELEASE)/wally-iod

# ***********************************************************************
# OBJECTS: debug
# ***********************************************************************

# Build dir for SRCS
BUILD_M_DEBUG            := $(BUILD)/debug

# Objects
OBJS_M_DEBUG             := $(patsubst $(SRC_DIR_M)/%.cpp, $(BUILD_M_DEBUG)/%.o, $(SRCS_M))

EXECOBJ_M_DEBUG          := $(patsubst $(SRC_DIR_M)/%.cpp, $(BUILD_M_DEBUG)/%.o, $(EXECSOURCE_M))

# Subdirs in build directory need to reflect subdirs in src directory
BUILDDIRS_M_DEBUG        := $(sort $(dir $(OBJS_M_DEBUG)))

# Main executable
EXEC_M_DEBUG             := $(BUILD_M_DEBUG)/wally-iod

# ***********************************************************************
# OBJECTS: sanitize
# ***********************************************************************

# Build dir for SRCS
BUILD_M_SAN              := $(BUILD)/sanitize

# Objects
OBJS_M_SAN               := $(patsubst $(SRC_DIR_M)/%.cpp, $(BUILD_M_SAN)/%.o, $(SRCS_M))

EXECOBJ_M_SAN            := $(patsubst $(SRC_DIR_M)/%.cpp, $(BUILD_M_SAN)/%.o, $(EXECSOURCE_M))

# Subdirs in build directory need to reflect subdirs in src directory
BUILDDIRS_M_SAN          := $(sort $(dir $(OBJS_M_SAN)))

# Main executable
EXEC_M_SAN               := $(BUILD_M_SAN)/wally-iod

# --------------------------------------------------------------------------------------------------------------------- 
# Unit tests
# --------------------------------------------------------------------------------------------------------------------- 

# Test SRCS
SRC_DIR_UT                := src/test/cpp

# Test framework SRCS
SRCS_UT_FRM               := $(wildcard $(SRC_DIR_UT)/framework/*.cpp)

# Test SRCS
SRCS_UT                   := $(wildcard $(SRC_DIR_UT)/io_wally/*.cpp)
SRCS_UT                   += $(wildcard $(SRC_DIR_UT)/io_wally/protocol/*.cpp)
SRCS_UT                   += $(wildcard $(SRC_DIR_UT)/io_wally/codec/*.cpp)
SRCS_UT                   += $(wildcard $(SRC_DIR_UT)/io_wally/impl/*.cpp)
SRCS_UT                   += $(wildcard $(SRC_DIR_UT)/io_wally/app/*.cpp)
SRCS_UT                   += $(wildcard $(SRC_DIR_UT)/io_wally/dispatch/*.cpp)

EXECSOURCE_UT             := $(wildcard $(SRC_DIR_UT)/tests_main.cpp)

# Build dir for tests
BUILD_UT                  := $(BUILD)/test

# Test framework objects
OBJS_UT_FRM               := $(patsubst $(SRC_DIR_UT)/%.cpp, $(BUILD_UT)/%.o, $(SRCS_UT_FRM))

# Test objects
OBJS_UT                   := $(patsubst $(SRC_DIR_UT)/%.cpp, $(BUILD_UT)/%.o, $(SRCS_UT))

EXECOBJ_UT                := $(patsubst $(SRC_DIR_UT)/%.cpp, $(BUILD_UT)/%.o, $(EXECSOURCE_UT))

# Subdirs in build directory need to reflect subdirs in test directory
BUILDDIRS_UT              := $(sort $(dir $(OBJS_UT)))
BUILDDIRS_UT              += $(dir $(OBJS_UT_FRM))

# Main test executable
EXEC_UT                   := $(BUILD_UT)/unit-tests

# --------------------------------------------------------------------------------------------------------------------- 
# Integration tests
# --------------------------------------------------------------------------------------------------------------------- 

SRC_DIR_IT                := src/itest/py

# --------------------------------------------------------------------------------------------------------------------- 
# Tooling
# --------------------------------------------------------------------------------------------------------------------- 

# Where to store scan-build's analysis results
BUILD_SCAN                := $(BUILD)/scan

# All things doxygen
BUILD_DOC                 := $(BUILD)/doc

# Clang's compilation database needed for some of its tooling
COMPILATIONDB             := compile_commands.json

# Third-party tools, mainly used for testing
TOOLSDIR                  := tools

# Paho conformance test suite
PAHO_CONFORM_DIR          := $(TOOLSDIR)/paho
PAHO_CONFORM_EXEC         := $(PAHO_CONFORM_DIR)/client_test.py

# --------------------------------------------------------------------------------------------------------------------- 
# Miscellaneous
# --------------------------------------------------------------------------------------------------------------------- 

# What may be rebuilt
REBUILDABLES              := $(EXEC_M) $(OBJS_M) $(EXEC_UT) $(OBJS_UT) $(EXEC_IT) $(OBJS_IT)

# --------------------------------------------------------------------------------------------------------------------- 
# Compiler configuration: common
# --------------------------------------------------------------------------------------------------------------------- 

# GCC
CXX                       := g++
#CXX                       := clang++
CC                        := gcc
#CC                        := clang

# --------------------------------------------------------------------------------------------------------------------- 
# Compiler configuration: main executable NORMAL
# --------------------------------------------------------------------------------------------------------------------- 

# Standard compiler flags
CXXFLAGS_M                := -std=c++14
CXXFLAGS_M                += -fdiagnostics-color=auto
CXXFLAGS_M                += -MMD # automatically generate dependency rules on each run
CXXFLAGS_M                += -I $(SRC_DIR_M)
CXXFLAGS_M                += -I $(BA_QUEUE_EXT_DIR)
CXXFLAGS_M                += -Werror
CXXFLAGS_M                += -Wall
CXXFLAGS_M                += -Wextra
CXXFLAGS_M                += -Wcast-align
CXXFLAGS_M                += -Wformat-nonliteral
CXXFLAGS_M                += -Wformat=2
CXXFLAGS_M                += -Winvalid-pch
CXXFLAGS_M                += -Wmissing-declarations
CXXFLAGS_M                += -Wmissing-format-attribute
CXXFLAGS_M                += -Wmissing-include-dirs
CXXFLAGS_M                += -Wredundant-decls
CXXFLAGS_M                += -Wswitch-default
CXXFLAGS_M                += -Wswitch-enum

# Standard preprocessor flags
CPPFLAGS_M                := -DBOOST_ALL_DYN_LINK
# Needed for clang:
# http://stackoverflow.com/questions/27552028/who-is-failing-boost-clang-or-gcc-issue-with-stdchrono-used-with-boostas
CPPFLAGS_M                += -DBOOST_ASIO_HAS_STD_CHRONO 

# Extra linker flags
LDLIBS_M                  := -lboost_system
LDLIBS_M                  += -lboost_thread
LDLIBS_M                  += -lboost_log
LDLIBS_M                  += -lboost_log_setup
LDLIBS_M                  += -lboost_program_options
LDLIBS_M                  += -lpthread

# --------------------------------------------------------------------------------------------------------------------- 
# Compiler configuration: main executable RELEASE
# --------------------------------------------------------------------------------------------------------------------- 

CXXFLAGS_REL              := $(CXXFLAGS_M)
CXXFLAGS_REL              += -O3

# --------------------------------------------------------------------------------------------------------------------- 
# Compiler configuration: main executable DEBUG (currently not working since we miss debug info for boost
# program_options)
# --------------------------------------------------------------------------------------------------------------------- 

CXXFLAGS_DEBUG            := $(CXXFLAGS_M)
CXXFLAGS_DEBUG            += -O0 -g
CXXFLAGS_DEBUG            += -DBOOST_ASIO_ENABLE_HANDLER_TRACKING
# Actually, we would like to enable _GLIBCXX_DEBUG, but this is incompatible with Boost Program Options's debug build.
# See: https://trac.macports.org/ticket/22112
#CXXFLAGS_DEBUG            += -D_GLIBCXX_DEBUG

# --------------------------------------------------------------------------------------------------------------------- 
# Compiler configuration: main executable SANITIZE
# --------------------------------------------------------------------------------------------------------------------- 

# Compiler/linker flags for checking programmer's sanity: copied from seastar's build
# See: http://btorpey.github.io/blog/2014/03/27/using-clangs-address-sanitizer/
CXXFLAGS_SAN              := $(CXXFLAGS_M)
CXXFLAGS_SAN              += -O0
CXXFLAGS_SAN              += -g # Needed by g++ to support line numbers in asan reports
CXXFLAGS_SAN              += -fsanitize=address
CXXFLAGS_SAN              += -fsanitize=leak
CXXFLAGS_SAN              += -fsanitize=undefined
CXXFLAGS_SAN              += -fno-sanitize=vptr # See: https://github.com/scylladb/seastar/issues/78
CXXFLAGS_SAN              += -fno-omit-frame-pointer

# Sanitizer linker flags
LDLIBS_SAN                := $(LDLIBS_M)
LDLIBS_SAN                += -fsanitize=address
LDLIBS_SAN                += -fsanitize=leak
LDLIBS_SAN                += -fsanitize=undefined
LDLIBS_SAN                += -fno-sanitize=vptr # See: https://github.com/scylladb/seastar/issues/78

# Correctly configure Address Sanitizer
# See: https://www.chromium.org/developers/testing/leaksanitizer
# See: http://tsdgeos.blogspot.de/2014/03/asan-and-gcc-how-to-get-line-numbers-in.html
# NOTE: a comment to the latter post suggests that recent g++ versions do not use llvm's symbolizer anymore, yet
# instead need sources compiled with debug flag set. THEREFORE, THESE OPTIONS ARE NOT USED ANYMORE.
# NOTE: I *believe* ASAN_OPTIONS is only used when *running* an application, *not* when compiling it.

ASAN_SUPPRESSIONS         := ./build/asan.supp
ASAN_OPTS                 := ASAN_OPTIONS=detect_leaks=1:symbolize=1:verbosity=0:suppressions=$(ASAN_SUPPRESSIONS)

# --------------------------------------------------------------------------------------------------------------------- 
# Compiler configuration: unit tests
# --------------------------------------------------------------------------------------------------------------------- 

# Test compiler flags
CXXFLAGS_UT               := $(CXXFLAGS_M)
CXXFLAGS_UT               += -O0
CXXFLAGS_UT               += -g # Needed by g++ to support line numbers in asan reports
CXXFLAGS_UT               += -I $(SRC_DIR_UT)
CXXFLAGS_UT               := $(filter-out -Wswitch-default, $(CXXFLAGS_UT))
CXXFLAGS_UT               := $(filter-out -Wswitch-enum, $(CXXFLAGS_UT))
CXXFLAGS_UT               += -fsanitize=address
CXXFLAGS_UT               += -fsanitize=leak
CXXFLAGS_UT               += -fsanitize=undefined
CXXFLAGS_UT               += -fno-sanitize=vptr # See: https://github.com/scylladb/seastar/issues/78
CXXFLAGS_UT               += -fno-omit-frame-pointer

# Integrationtest preprocessor flags
CPPFLAGS_UT               := $(CPPFLAGS_M)

# Test linker flags
LDLIBS_UT                 := $(LDLIBS_M)
LDLIBS_UT                 += -fsanitize=address
LDLIBS_UT                 += -fsanitize=leak
LDLIBS_UT                 += -fsanitize=undefined
LDLIBS_UT                 += -fno-sanitize=vptr # See: https://github.com/scylladb/seastar/issues/78

# --------------------------------------------------------------------------------------------------------------------- 
# catch.hpp: test library on github
# --------------------------------------------------------------------------------------------------------------------- 

CATCH_DL_URL              := https://raw.githubusercontent.com/philsquared/Catch/master/single_include/catch.hpp

#######################################################################################################################
# Rules
#######################################################################################################################

# --------------------------------------------------------------------------------------------------------------------- 
# Build main executable NORMAL
# --------------------------------------------------------------------------------------------------------------------- 

main                      : $(EXEC_M)                              | $(BUILDDIRS_M)

$(BUILDDIRS_M)            :
	@mkdir -p $@

$(EXEC_M)                 : $(OBJS_M) $(EXECOBJ_M)                 | $(BUILDDIRS_M)
	$(CXX) $(LDLIBS_M) -o $@ $^

$(BUILD_M)/%.o            : $(SRC_DIR_M)/%.cpp                     | $(BUILDDIRS_M)
	$(CXX) $(CPPFLAGS_M) $(CXXFLAGS_M) -o $@ -c $<

# --------------------------------------------------------------------------------------------------------------------- 
# Build main executable RELEASE
# --------------------------------------------------------------------------------------------------------------------- 

release                   : $(EXEC_M_RELEASE)                      | $(BUILDDIRS_M_RELEASE)

$(BUILDDIRS_M_RELEASE)    :
	@mkdir -p $@

$(EXEC_M_RELEASE)         : $(OBJS_M_RELEASE) $(EXECOBJ_M_RELEASE) | $(BUILDDIRS_M_RELEASE)
	$(CXX) $(LDLIBS_M) -o $@ $^

$(BUILD_M_RELEASE)/%.o    : $(SRC_DIR_M)/%.cpp                     | $(BUILDDIRS_M_RELEASE)
	$(CXX) $(CPPFLAGS_M) $(CXXFLAGS_REL) -o $@ -c $<

# --------------------------------------------------------------------------------------------------------------------- 
# Build main executable DEBUG
# --------------------------------------------------------------------------------------------------------------------- 

debug                     : $(EXEC_M_DEBUG)                        | $(BUILDDIRS_M_DEBUG)

$(BUILDDIRS_M_DEBUG)      :
	@mkdir -p $@

$(EXEC_M_DEBUG)           : $(OBJS_M_DEBUG) $(EXECOBJ_M_DEBUG)     | $(BUILDDIRS_M_DEBUG)
	$(CXX) $(LDLIBS_M) -o $@ $^

$(BUILD_M_DEBUG)/%.o      : $(SRC_DIR_M)/%.cpp                     | $(BUILDDIRS_M_DEBUG)
	$(CXX) $(CPPFLAGS_M) $(CXXFLAGS_DEBUG) -o $@ -c $<

# --------------------------------------------------------------------------------------------------------------------- 
# Build main executable SANITIZE
# --------------------------------------------------------------------------------------------------------------------- 

sanitize                  : $(EXEC_M_SAN)                           | $(BUILDDIRS_M_SAN)

$(BUILDDIRS_M_SAN)        :
	@mkdir -p $@

$(EXEC_M_SAN)             : $(OBJS_M_SAN) $(EXECOBJ_M_SAN)          | $(BUILDDIRS_M_SAN)
	$(CXX) $(LDLIBS_SAN) -o $@ $^

$(BUILD_M_SAN)/%.o        : $(SRC_DIR_M)/%.cpp                      | $(BUILDDIRS_M_SAN)
	$(CXX) $(CPPFLAGS_M) $(CXXFLAGS_SAN) -o $@ -c $<

# --------------------------------------------------------------------------------------------------------------------- 
# Build/run unit tests
# --------------------------------------------------------------------------------------------------------------------- 

.PHONY                    : test
test                      : test-compile
	@./$(EXEC_UT)

.PHONY                    : test-success
test-success              : test-compile
	@./$(EXEC_UT) --success --durations yes

test-compile              : $(EXEC_UT)                             | $(BUILDDIRS_UT)

$(BUILDDIRS_UT)           :
	@mkdir -p $@

$(EXEC_UT)                : $(OBJS_M_SAN) $(OBJS_UT) $(OBJS_UT_FRM) $(EXECOBJ_UT) | $(BUILDDIRS_UT)
	$(CXX) $(LDLIBS_UT) -o $@ $^

$(BUILD_UT)/%.o           : $(SRC_DIR_UT)/%.cpp                    | $(BUILDDIRS_UT)
	$(CXX) $(CPPFLAGS_UT) $(CXXFLAGS_UT) -o $@ -c $<

# --------------------------------------------------------------------------------------------------------------------- 
# Build/run integration tests
# --------------------------------------------------------------------------------------------------------------------- 

.PHONY                    : itest
itest                     : sanitize
	@python3 -m unittest discover --verbose --start-directory $(SRC_DIR_IT) --pattern "*_tests.py"

# --------------------------------------------------------------------------------------------------------------------- 
# Build/run load tests
# --------------------------------------------------------------------------------------------------------------------- 

.PHONY                    : ltest
ltest                     : main
	@python3 ./src/ltest/py/simple_load_test.py

# --------------------------------------------------------------------------------------------------------------------- 
# Paho conformance test suit
# --------------------------------------------------------------------------------------------------------------------- 

.PHONY                    : conformance-tests
conformance-tests         :
	@python3 $(PAHO_CONFORM_EXEC)

# --------------------------------------------------------------------------------------------------------------------- 
# Clean up the mess
# --------------------------------------------------------------------------------------------------------------------- 

.PHONY                    : clean
clean                     :
	@rm -rf $(BUILD)

# --------------------------------------------------------------------------------------------------------------------- 
# Generate/publish documentation
# --------------------------------------------------------------------------------------------------------------------- 

.PHONY                    : doc
doc                       : $(SRCS_M) $(EXECSOURCE_M)
	@rm -rf $(BUILD_DOC)
	@doxygen ./build/doxygen.cfg

.PHONY                    : doc-publish
doc-publish               : doc
	@pushd $(BUILD_DOC); python -mSimpleHTTPServer 8000; popd

# --------------------------------------------------------------------------------------------------------------------- 
# Tooling
# --------------------------------------------------------------------------------------------------------------------- 

compilation-db            : $(COMPILATIONDB)

$(COMPILATIONDB)          : $(SRCS_M) $(EXECSOURCE_M) $(SRCS_UT) $(EXECSOURCE_UT) $(SRCS_IT) $(EXECSOURCE_IT)
	@bear --cdb $(COMPILATIONDB) $(MAKE) clean main test-compile

.PHONY                    : macroexpand
macroexpand               : $(SRCS_M) $(EXECSOURCE_M)
	$(CXX) $(CXXFLAGS_M) -E $(SRCS_M) | source-highlight --failsafe --src-lang=cc -f esc --style-file=esc.style 

# Running clang-check
.PHONY                    : check-main
check-main                : compilation-db
check-main                : $(SRCS_M) $(EXECSOURCE_M)
	@clang-check -p=$(COMPILATIONDB) $(SRCS_M) $(EXECSOURCE_M)

.PHONY                    : check-test
check-test                : compilation-db
check-test                : $(SRCS_UT) $(EXECSOURCE_UT)
	@clang-check -p=$(COMPILATIONDB) $(SRCS_UT) $(EXECSOURCE_UT)

.PHONY                    : check
check                     : check-main check-test 

# Running scan-build
$(BUILD_SCAN)             :
	@mkdir -p $@

.PHONY                    : scan-main
scan-main                 : $(BUILD_SCAN)
	@scan-build -o $(BUILD_SCAN) -analyze-headers --status-bugs $(MAKE) clean main

.PHONY                    : modernize
modernize                 : $(SRCS_M) $(EXECSOURCE_M) $(COMPILATIONDB)
	@clang-modernize -final-syntax-check -summary -format -style=file -include=src/ -p $(COMPILATIONDB)

.PHONY                    : format-main
format-main               : $(SRCS_M) $(EXECSOURCE_M)
	@clang-format -i -style=file $(SRCS_M) $(EXECSOURCE_M)

.PHONY                    : format-test
format-test               : $(SRCS_UT) $(EXECSOURCE_UT)
	@clang-format -i -style=file $(SRCS_UT) $(EXECSOURCE_UT)

.PHONY                    : format
format                    : format-main
format                    : format-test

.PHONY                    : tags
tags                      : $(SRCS_M) $(EXECSOURCE_M) $(SRCS_UT) $(EXECSOURCE_UT)
	@ctags -R -f ./.tags $(SRC_DIR_M) $(SRC_DIR_UT)

.PHONY                    : upgrade-catch-hpp
upgrade-catch-hpp         :
	@mv $(SRC_DIR_UT)/catch.hpp $(SRC_DIR_UT)/catch.hpp.backup; \
		curl --progress-bar --output $(SRC_DIR_UT)/catch.hpp $(CATCH_DL_URL);

# --------------------------------------------------------------------------------------------------------------------- 
# Prepare a commit: run (almost) everything
# --------------------------------------------------------------------------------------------------------------------- 

.PHONY                    : prepare-commit
prepare-commit            : clean
prepare-commit            : format
prepare-commit            : sanitize
prepare-commit            : test 
prepare-commit            : itest 
prepare-commit            : check
prepare-commit            : scan-main

# --------------------------------------------------------------------------------------------------------------------- 
# Helpers
# --------------------------------------------------------------------------------------------------------------------- 

# Run server with some convenient default settings
.PHONY                    : run-server
run-server                : $(EXEC_M)
	@$(EXEC_M) --log-file .testlog --log-file-level trace --log-console --log-console-level trace --conn-timeout 1000000

# Run server with some convenient default settings, this time using a binary instrumented by Clang's sanitizers
.PHONY                    : run-server-san
run-server-san            : $(EXEC_M_SAN)
	@$(EXEC_M_SAN) --log-file .testlog --log-file-level trace --log-console --log-console-level trace --conn-timeout 1000000

#######################################################################################################################
# Dependency rules: http://stackoverflow.com/questions/8025766/makefile-auto-dependency-generation
#######################################################################################################################

-include                  $(OBJS_M:.o=.d)
-include                  $(EXECOBJ_M:.o=.d)
-include                  $(OBJS_M_RELEASE:.o=.d)
-include                  $(EXECOBJ_M_RELEASE:.o=.d)
-include                  $(OBJS_M_DEBUG:.o=.d)
-include                  $(EXECOBJ_M_DEBUG:.o=.d)
-include                  $(OBJS_M_SAN:.o=.d)
-include                  $(EXECOBJ_M_SAN:.o=.d)
-include                  $(OBJS_UT:.o=.d)
-include                  $(EXECOBJ_UT:.o=.d)
