#######################################################################################################################
# Includes
#######################################################################################################################

include ./external/external.mk

#######################################################################################################################
# Command line arguments
#######################################################################################################################

# run|release|debug|asan
config                   ?= asan

SUPPORTED_CONFIGS        := run release debug asan

ifeq ($(filter $(config),$(SUPPORTED_CONFIGS)),)
$(error Unsupported config = $(config) - supported configs are: $(SUPPORTED_CONFIGS))
endif

# gcc|clang
toolchain                ?= gcc

SUPPORTED_TOOLCHAINS     := gcc clang

ifeq ($(filter $(toolchain),$(SUPPORTED_TOOLCHAINS)),)
$(error Unsupported toolchain = $(toolchain) - supported toolchains are: $(SUPPORTED_TOOLCHAINS))
endif

# normal|scan
mode                     ?= normal

SUPPORTED_MODES          := normal scan

ifeq ($(filter $(mode),$(SUPPORTED_MODES)),)
$(error Unsupported mode = $(mode) - supported modes are: $(SUPPORTED_MODES))
endif

#######################################################################################################################
# Macros
#######################################################################################################################

# --------------------------------------------------------------------------------------------------------------------- 
# Common definitions
# --------------------------------------------------------------------------------------------------------------------- 

# http://stackoverflow.com/questions/18136918/how-to-get-current-directory-of-your-makefile
DIR                      := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

# Top level build directory
# see: http://blog.kompiler.org/post/6/2011-09-18/Separate_build_and_source_directories_in_Makefiles/
BUILD_BASE               := $(DIR)/target
BUILD                    := $(BUILD_BASE)/$(config)

# --------------------------------------------------------------------------------------------------------------------- 
# Main executable: sources
# --------------------------------------------------------------------------------------------------------------------- 

SRC_DIR_M                := $(DIR)/source/server

SRCS_M                   := $(wildcard $(SRC_DIR_M)/io_wally/*.cpp)
SRCS_M                   += $(wildcard $(SRC_DIR_M)/io_wally/protocol/*.cpp)
SRCS_M                   += $(wildcard $(SRC_DIR_M)/io_wally/codec/*.cpp)
SRCS_M                   += $(wildcard $(SRC_DIR_M)/io_wally/spi/*.cpp)
SRCS_M                   += $(wildcard $(SRC_DIR_M)/io_wally/impl/*.cpp)
SRCS_M                   += $(wildcard $(SRC_DIR_M)/io_wally/dispatch/*.cpp)
SRCS_M                   += $(wildcard $(SRC_DIR_M)/io_wally/logging/*.cpp)
SRCS_M                   += $(wildcard $(SRC_DIR_M)/io_wally/app/*.cpp)

EXECSOURCE_M             := $(wildcard $(SRC_DIR_M)/*.cpp)

# --------------------------------------------------------------------------------------------------------------------- 
# Main executable: objects
# --------------------------------------------------------------------------------------------------------------------- 

# Build dir
BUILD_M                  := $(BUILD)/main

# Objects
OBJS_M                   := $(patsubst $(SRC_DIR_M)/%.cpp, $(BUILD_M)/%.o, $(SRCS_M))

EXECOBJ_M                := $(patsubst $(SRC_DIR_M)/%.cpp, $(BUILD_M)/%.o, $(EXECSOURCE_M))

# Subdirs in build directory need to reflect subdirs in src directory
BUILDDIRS_M              := $(sort $(dir $(OBJS_M)))

# Main executable
EXEC_M                   := $(BUILD_M)/wally-iod

# --------------------------------------------------------------------------------------------------------------------- 
# Unit tests: sources
# --------------------------------------------------------------------------------------------------------------------- 

# Test SRCS
SRC_DIR_UT                := $(DIR)/test/server

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

# --------------------------------------------------------------------------------------------------------------------- 
# Unit tests: objects
# --------------------------------------------------------------------------------------------------------------------- 

# Build dir
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
# Unit tests: test reports
# --------------------------------------------------------------------------------------------------------------------- 

# Build dir
BUILD_UT_REPORTS          := $(BUILD)/test-reports

# --------------------------------------------------------------------------------------------------------------------- 
# Integration tests
# --------------------------------------------------------------------------------------------------------------------- 

SRC_DIR_IT                := $(DIR)/itest/server

# Build dir for integration tests
BUILD_IT                  := $(BUILD)/itest

# --------------------------------------------------------------------------------------------------------------------- 
# Load tests
# --------------------------------------------------------------------------------------------------------------------- 

SRC_DIR_LT                := $(DIR)/ltest/server

# --------------------------------------------------------------------------------------------------------------------- 
# Tooling
# --------------------------------------------------------------------------------------------------------------------- 

# Where scan-build/clang analyzer stores its HTML reports
BUILD_SCAN                := $(BUILD)/scan
BUILD_M_SCAN              := $(BUILD_SCAN)/main
BUILD_UT_SCAN             := $(BUILD_SCAN)/test

# All things doxygen
BUILD_DOC                 := $(BUILD)/doc

# Clang's compilation database needed for some of its tooling
COMPILATIONDB             := $(DIR)/compile_commands.json

# Third-party tools, mainly used for testing
TOOLSDIR                  := $(DIR)/tools

# --------------------------------------------------------------------------------------------------------------------- 
# Miscellaneous
# --------------------------------------------------------------------------------------------------------------------- 

# What may be rebuilt
REBUILDABLES              := $(EXEC_M) $(OBJS_M) $(EXEC_UT) $(OBJS_UT) $(EXEC_IT) $(OBJS_IT)

# --------------------------------------------------------------------------------------------------------------------- 
# Compiler configuration: common
# --------------------------------------------------------------------------------------------------------------------- 

# Choose compiler based on command line parameter 'toolchain'
ifeq ($(toolchain),gcc)
CXX := g++
CC  := gcc
else
CXX := clang++
CC  := clang
endif

# --------------------------------------------------------------------------------------------------------------------- 
# Compiler configuration/main: config = run
# --------------------------------------------------------------------------------------------------------------------- 

# Standard compiler flags
CXXFLAGS_RUN              := -std=c++17
CXXFLAGS_RUN              += -c # Only compile
CXXFLAGS_RUN              += -fdiagnostics-color=auto
CXXFLAGS_RUN              += -MMD # automatically generate dependency rules on each run
CXXFLAGS_RUN              += -I $(SRC_DIR_M)
CXXFLAGS_RUN              += -I $(ASIO_EXT_INC)
CXXFLAGS_RUN              += -I $(SPDLOG_EXT_INC)
CXXFLAGS_RUN              += -I $(CXXOPTS_EXT_INC)
CXXFLAGS_RUN              += -Werror
CXXFLAGS_RUN              += -Wall
CXXFLAGS_RUN              += -Wextra
CXXFLAGS_RUN              += -Wcast-align
CXXFLAGS_RUN              += -Wformat-nonliteral
CXXFLAGS_RUN              += -Wformat=2
CXXFLAGS_RUN              += -Winvalid-pch
CXXFLAGS_RUN              += -Wmissing-declarations
CXXFLAGS_RUN              += -Wmissing-format-attribute
CXXFLAGS_RUN              += -Wmissing-include-dirs
CXXFLAGS_RUN              += -Wredundant-decls
CXXFLAGS_RUN              += -Wswitch-default
CXXFLAGS_RUN              += -Wswitch-enum

# Standard preprocessor flags
CPPFLAGS_RUN              := -DASIO_STANDALONE
# Needed for clang?
CPPFLAGS_RUN              += -DASIO_HAS_STD_CHRONO

# Extra linker flags
LDLIBS_RUN                += -lpthread

# --------------------------------------------------------------------------------------------------------------------- 
# Compiler configuration/main: config = release
# --------------------------------------------------------------------------------------------------------------------- 

CXXFLAGS_REL              := $(CXXFLAGS_RUN)
CXXFLAGS_REL              += -O3

# Release build preprocessor flags
CPPFLAGS_REL              := $(CPPFLAGS_RUN)

# Release linker flags
LDLIBS_REL                := $(LDLIBS_RUN)

# --------------------------------------------------------------------------------------------------------------------- 
# Compiler configuration/main: config = debug
# --------------------------------------------------------------------------------------------------------------------- 

CXXFLAGS_DEBUG            := $(CXXFLAGS_RUN)
CXXFLAGS_DEBUG            += -O0 -g
#CXXFLAGS_DEBUG            += -DASIO_ENABLE_HANDLER_TRACKING
CXXFLAGS_DEBUG            += -D_GLIBCXX_DEBUG

# Debug build preprocessor flags
CPPFLAGS_DEBUG            := $(CPPFLAGS_RUN)

# Debug linker flags
LDLIBS_DEBUG              := $(LDLIBS_RUN)

# --------------------------------------------------------------------------------------------------------------------- 
# Compiler configuration/main: config = asan
# --------------------------------------------------------------------------------------------------------------------- 

# Compiler/linker flags for checking programmer's sanity: copied from seastar's build
# See: http://btorpey.github.io/blog/2014/03/27/using-clangs-address-sanitizer/
CXXFLAGS_ASAN             := $(CXXFLAGS_RUN)
CXXFLAGS_ASAN             += -O0
CXXFLAGS_ASAN             += -g # Needed by g++ to support line numbers in asan reports
CXXFLAGS_ASAN             += -fsanitize=address
CXXFLAGS_ASAN             += -fsanitize=leak
CXXFLAGS_ASAN             += -fsanitize=undefined
CXXFLAGS_ASAN             += -fno-sanitize=vptr # See: https://github.com/scylladb/seastar/issues/78
CXXFLAGS_ASAN             += -fno-omit-frame-pointer

# Sanitize build preprocessor flags
CPPFLAGS_ASAN             := $(CPPFLAGS_RUN)

# Sanitizer linker flags
LDLIBS_ASAN               := $(LDLIBS_RUN)
LDLIBS_ASAN               += -fsanitize=address
LDLIBS_ASAN               += -fsanitize=leak
LDLIBS_ASAN               += -fsanitize=undefined
LDLIBS_ASAN               += -fno-sanitize=vptr # See: https://github.com/scylladb/seastar/issues/78

# Correctly configure Address Sanitizer
# See: https://www.chromium.org/developers/testing/leaksanitizer
# See: http://tsdgeos.blogspot.de/2014/03/asan-and-gcc-how-to-get-line-numbers-in.html
# NOTE: a comment to the latter post suggests that recent g++ versions do not use llvm's symbolizer anymore, yet
# instead need sources compiled with debug flag set. THEREFORE, THESE OPTIONS ARE NOT USED ANYMORE.
# NOTE: I *believe* ASAN_OPTIONS is only used when *running* an application, *not* when compiling it.

ASAN_SUPPRESSIONS         := ./build/asan.supp
ASAN_OPTS                 := ASAN_OPTIONS=detect_leaks=1:symbolize=1:verbosity=0:suppressions=$(ASAN_SUPPRESSIONS)

# --------------------------------------------------------------------------------------------------------------------- 
# Compiler configuration/test: config = run
# --------------------------------------------------------------------------------------------------------------------- 

# Test compiler flags
CXXFLAGS_UT_RUN           := $(CXXFLAGS_RUN)
CXXFLAGS_UT_RUN           += -I $(SRC_DIR_UT)
CXXFLAGS_UT_RUN           += -I $(CATCH2_EXT_INC)
CXXFLAGS_UT_RUN           := $(filter-out -Wswitch-default, $(CXXFLAGS_UT_RUN))
CXXFLAGS_UT_RUN           := $(filter-out -Wswitch-enum, $(CXXFLAGS_UT_RUN))

# Unit test preprocessor flags
CPPFLAGS_UT_RUN           := $(CPPFLAGS_RUN)

# Test linker flags
LDLIBS_UT_RUN             := $(LDLIBS_RUN)

# --------------------------------------------------------------------------------------------------------------------- 
# Compiler configuration/test: config = release
# --------------------------------------------------------------------------------------------------------------------- 

# Test compiler flags
CXXFLAGS_UT_REL           := $(CXXFLAGS_UT_RUN)

# Unit test preprocessor flags
CPPFLAGS_UT_REL           := $(CPPFLAGS_UT_RUN)

# Test linker flags
LDLIBS_UT_REL             := $(LDLIBS_UT_RUN)

# --------------------------------------------------------------------------------------------------------------------- 
# Compiler configuration/test: config = debug
# --------------------------------------------------------------------------------------------------------------------- 

# Test compiler flags
CXXFLAGS_UT_DEBUG         := $(CXXFLAGS_UT_RUN)
CXXFLAGS_UT_DEBUG         += -O0
CXXFLAGS_UT_DEBUG         += -g # Needed by g++ to support line numbers in asan reports

# Unit test preprocessor flags
CPPFLAGS_UT_DEBUG         := $(CPPFLAGS_UT_RUN)

# Test linker flags
LDLIBS_UT_DEBUG           := $(LDLIBS_UT_RUN)

# --------------------------------------------------------------------------------------------------------------------- 
# Compiler configuration/test: config = asan
# --------------------------------------------------------------------------------------------------------------------- 

# Test compiler flags
CXXFLAGS_UT_ASAN          := $(CXXFLAGS_UT_RUN)
CXXFLAGS_UT_ASAN          += -O0
CXXFLAGS_UT_ASAN          += -g # Needed by g++ to support line numbers in asan reports
CXXFLAGS_UT_ASAN          += -fsanitize=address
CXXFLAGS_UT_ASAN          += -fsanitize=leak
CXXFLAGS_UT_ASAN          += -fsanitize=undefined
CXXFLAGS_UT_ASAN          += -fno-sanitize=vptr # See: https://github.com/scylladb/seastar/issues/78
CXXFLAGS_UT_ASAN          += -fno-omit-frame-pointer

# Unit test preprocessor flags
CPPFLAGS_UT_ASAN          := $(CPPFLAGS_UT_RUN)

# Test linker flags
LDLIBS_UT_ASAN            := $(LDLIBS_UT_RUN)
LDLIBS_UT_ASAN            += -fsanitize=address
LDLIBS_UT_ASAN            += -fsanitize=leak
LDLIBS_UT_ASAN            += -fsanitize=undefined
LDLIBS_UT_ASAN            += -fno-sanitize=vptr # See: https://github.com/scylladb/seastar/issues/78

# --------------------------------------------------------------------------------------------------------------------- 
# Select compiler flags based on command line param 'config'
# --------------------------------------------------------------------------------------------------------------------- 

ifeq ($(config),run)
CXXFLAGS_M                := $(CXXFLAGS_RUN)
CPPFLAGS_M                := $(CPPFLAGS_RUN)
LDLIBS_M                  := $(LDLIBS_RUN)
CXXFLAGS_UT               := $(CXXFLAGS_UT_RUN)
CPPFLAGS_UT               := $(CPPFLAGS_UT_RUN)
LDLIBS_UT                 := $(LDLIBS_UT_RUN)
endif
ifeq ($(config),debug)
CXXFLAGS_M                := $(CXXFLAGS_DEBUG)
CPPFLAGS_M                := $(CPPFLAGS_DEBUG)
LDLIBS_M                  := $(LDLIBS_DEBUG)
CXXFLAGS_UT               := $(CXXFLAGS_UT_DEBUG)
CPPFLAGS_UT               := $(CPPFLAGS_UT_DEBUG)
LDLIBS_UT                 := $(LDLIBS_UT_DEBUG)
endif
ifeq ($(config),release)
CXXFLAGS_M                := $(CXXFLAGS_REL)
CPPFLAGS_M                := $(CPPFLAGS_REL)
LDFLAGS_M                 := -static -static-libstdc++
LDLIBS_M                  := $(LDLIBS_REL)
CXXFLAGS_UT               := $(CXXFLAGS_UT_REL)
CPPFLAGS_UT               := $(CPPFLAGS_UT_REL)
LDLIBS_UT                 := $(LDLIBS_UT_REL)
endif
ifeq ($(config),asan)
CXXFLAGS_M                := $(CXXFLAGS_ASAN)
CPPFLAGS_M                := $(CPPFLAGS_ASAN)
LDLIBS_M                  := $(LDLIBS_ASAN)
CXXFLAGS_UT               := $(CXXFLAGS_UT_ASAN)
CPPFLAGS_UT               := $(CPPFLAGS_UT_ASAN)
LDLIBS_UT                 := $(LDLIBS_UT_ASAN)
endif

#######################################################################################################################
# Rules
#######################################################################################################################

# --------------------------------------------------------------------------------------------------------------------- 
# Convenience rules
# --------------------------------------------------------------------------------------------------------------------- 

main-run                  :
	$(MAKE) config=run toolchain=$(toolchain) mode=$(mode) main

main-run-scan             :
	$(MAKE) config=run toolchain=$(toolchain) mode=scan main

main-debug                :
	$(MAKE) config=debug toolchain=$(toolchain) mode=$(mode) main

main-debug-scan           :
	$(MAKE) config=debug toolchain=$(toolchain) mode=scan main

main-release              :
	$(MAKE) config=release toolchain=$(toolchain) mode=$(mode) main

main-release-scan         :
	$(MAKE) config=release toolchain=$(toolchain) mode=scan main

main-asan                 :
	$(MAKE) config=asan toolchain=$(toolchain) mode=$(mode) main

main-asan-scan            :
	$(MAKE) config=asan toolchain=$(toolchain) mode=scan main

test-run                  :
	$(MAKE) config=run toolchain=$(toolchain) mode=$(mode) test

test-run-scan             :
	$(MAKE) config=run toolchain=$(toolchain) mode=scan test

test-debug                :
	$(MAKE) config=debug toolchain=$(toolchain) mode=$(mode) test

test-debug-scan           :
	$(MAKE) config=debug toolchain=$(toolchain) mode=scan test

test-release              :
	$(MAKE) config=release toolchain=$(toolchain) mode=$(mode) test

test-release-scan         :
	$(MAKE) config=release toolchain=$(toolchain) mode=scan test

test-asan                 :
	$(MAKE) config=asan toolchain=$(toolchain) mode=$(mode) test

test-asan-scan            :
	$(MAKE) config=asan toolchain=$(toolchain) mode=scan test

# --------------------------------------------------------------------------------------------------------------------- 
# Prepare a commit: run (almost) everything
# --------------------------------------------------------------------------------------------------------------------- 

.PHONY                    : prepare-commit
prepare-commit            : clean
prepare-commit            : format
prepare-commit            : main-asan-scan
prepare-commit            : test-asan-scan 
prepare-commit            : itest 
prepare-commit            : check

# --------------------------------------------------------------------------------------------------------------------- 
# Clean up the mess
# --------------------------------------------------------------------------------------------------------------------- 

.PHONY                    : clean
clean                     :
	@rm -rf $(BUILD)

.PHONY                    : clean-all
clean-all                 :
	@rm -rf $(BUILD_BASE)

# --------------------------------------------------------------------------------------------------------------------- 
# Build main executable
# --------------------------------------------------------------------------------------------------------------------- 

main-compile              : $(EXEC_M)                              | $(BUILDDIRS_M)

$(BUILDDIRS_M)            :
	@mkdir -p $@

$(BUILD_M)/%.o            : $(SRC_DIR_M)/%.cpp                     | $(BUILDDIRS_M)
	$(CXX) $(CPPFLAGS_M) $(CXXFLAGS_M) -o $@ -c $<

$(EXEC_M)                 : $(OBJS_M) $(EXECOBJ_M)                 | $(BUILDDIRS_M)
	$(CXX) $(LDFLAGS_M) -o $@ $^ $(LDLIBS_M)

ifeq ($(mode),normal)
main                      : main-compile
else
main                      : main-scan                           

$(BUILD_M_SCAN)             :
	@mkdir -p $@

main-scan                 : $(BUILD_M_SCAN)
	scan-build -o $(BUILD_M_SCAN) \
			   --status-bugs \
			   -plist-html \
			   $(MAKE) config=$(config) toolchain=$(toolchain) main-compile
endif

# --------------------------------------------------------------------------------------------------------------------- 
# Build unit tests
# --------------------------------------------------------------------------------------------------------------------- 

test-comp                 : $(EXEC_UT)                             | $(BUILDDIRS_UT)

$(BUILDDIRS_UT)           :
	@mkdir -p $@

$(BUILD_UT)/%.o           : $(SRC_DIR_UT)/%.cpp                    | $(BUILDDIRS_UT)
	$(CXX) $(CPPFLAGS_UT) $(CXXFLAGS_UT) -o $@ -c $<

$(EXEC_UT)                : $(OBJS_M) $(OBJS_UT) $(OBJS_UT_FRM) $(EXECOBJ_UT) | $(BUILDDIRS_UT)
	$(CXX) -o $@ $^ $(LDLIBS_UT)

ifeq ($(mode),normal)
test-compile              : test-comp
else
test-compile              : test-comp-scan

$(BUILD_UT_SCAN)          :
	@mkdir -p $@

test-comp-scan            : $(BUILD_UT_SCAN)
	scan-build -o $(BUILD_UT_SCAN) \
			   --status-bugs \
			   -plist-html \
			   $(MAKE) config=$(config) toolchain=$(toolchain) test-comp
endif

# --------------------------------------------------------------------------------------------------------------------- 
# Run unit tests
# --------------------------------------------------------------------------------------------------------------------- 

$(BUILD_UT_REPORTS)       :
	@mkdir -p $@

.PHONY                    : test
test                      : test-compile                       
	@$(EXEC_UT) --reporter console

.PHONY                    : test-success
test-success              : test-compile
	@$(EXEC_UT) --success --durations yes

.PHONY                    : test-report
test-report               : test-compile                           | $(BUILD_UT_REPORTS)
	@$(EXEC_UT) --reporter xml --out $(BUILD_UT_REPORTS)/catch2-report.xml

# --------------------------------------------------------------------------------------------------------------------- 
# Build/run integration tests
# --------------------------------------------------------------------------------------------------------------------- 

$(BUILD_IT)               :
	@mkdir -p $@

.PHONY                    : itest
itest                     : main                                   | $(BUILD_IT)
	@CONFIG=$(config) python3 -m unittest discover --verbose --start-directory $(SRC_DIR_IT) --pattern "*_tests.py"

# --------------------------------------------------------------------------------------------------------------------- 
# Build/run load tests
# --------------------------------------------------------------------------------------------------------------------- 

.PHONY                    : ltest
ltest                     : main
	@CONFIG=$(config) python3 $(SRC_DIR_LT)/simple_load_test.py

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

$(COMPILATIONDB)          : $(SRCS_M) $(EXECSOURCE_M) $(SRCS_UT) $(EXECSOURCE_UT)
	@bear --cdb $(COMPILATIONDB) $(MAKE) clean main-compile test-compile

.PHONY                    : macroexpand
macroexpand               : $(SRCS_M) $(EXECSOURCE_M)
	$(CXX) $(CXXFLAGS_M) -E $(SRCS_M) | source-highlight --failsafe --src-lang=cc -f esc --style-file=esc.style 

# Running clang-check
.PHONY                    : check-main
check-main                : compilation-db
check-main                : $(SRCS_M) $(EXECSOURCE_M)
	clang-check -p=$(COMPILATIONDB) $(SRCS_M) $(EXECSOURCE_M)

.PHONY                    : check-test
check-test                : compilation-db
check-test                : $(SRCS_UT) $(EXECSOURCE_UT)
	clang-check -p=$(COMPILATIONDB) $(SRCS_UT) $(EXECSOURCE_UT)

.PHONY                    : check
check                     : check-main
check                     : check-test 

.PHONY                    : tidy
tidy                      : compilation-db
tidy                      : $(SRCS_M) $(EXECSOURCE_M) $(COMPILATIONDB)
	clang-tidy -p=$(COMPILATIONDB) \
		-config= \
		-export-fixes=$(BUILD_BASE)/clang-tidy-fixes.yaml \
		$(SRCS_M) $(EXECSOURCE_M) \
		$(SRCS_UT) $(EXECSOURCE_UT)

.PHONY                    : format-main
format-main               : $(SRCS_M) $(EXECSOURCE_M)
	clang-format -i -style=file $(SRCS_M) $(EXECSOURCE_M)

.PHONY                    : format-test
format-test               : $(SRCS_UT) $(EXECSOURCE_UT)
	clang-format -i -style=file $(SRCS_UT) $(EXECSOURCE_UT)

.PHONY                    : format-external
format-external           : $(EXT_SRCS)
	clang-format -i -style=file $(EXT_SRCS)

.PHONY                    : format
format                    : format-main
format                    : format-test
format                    : format-external

.PHONY                    : tags
tags                      : $(SRCS_M) $(EXECSOURCE_M) $(SRCS_UT) $(EXECSOURCE_UT) $(EXT_SRCS)
	ctags -R -f ./.tags $(SRC_DIR_M) $(SRC_DIR_UT) $(EXT_SRCS)

# --------------------------------------------------------------------------------------------------------------------- 
# Helpers
# --------------------------------------------------------------------------------------------------------------------- 

# Run server with some convenient default settings
.PHONY                    : run-server
run-server                : $(EXEC_M)
	@$(EXEC_M) --log-file .testlog --log-level trace --log-console --conn-timeout 1000000

#######################################################################################################################
# Dependency rules: http://stackoverflow.com/questions/8025766/makefile-auto-dependency-generation
#######################################################################################################################

-include                  $(OBJS_M:.o=.d)
-include                  $(EXECOBJ_M:.o=.d)
-include                  $(OBJS_UT:.o=.d)
-include                  $(OBJS_UT_FRM:.o=.d)
-include                  $(EXECOBJ_UT:.o=.d)

