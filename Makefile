#######################################################################################################################
# Macros
#######################################################################################################################

# --------------------------------------------------------------------------------------------------------------------- 
# Common definitions
# --------------------------------------------------------------------------------------------------------------------- 

# Top level build directory
# see: http://blog.kompiler.org/post/6/2011-09-18/Separate_build_and_source_directories_in_Makefiles/
BUILD            := target

# --------------------------------------------------------------------------------------------------------------------- 
# Main executable: sources
# --------------------------------------------------------------------------------------------------------------------- 

#
# SOURCES
#

SRCS_M           := $(wildcard src/io_wally/*.cpp)
SRCS_M           += $(wildcard src/io_wally/protocol/*.cpp)
SRCS_M           += $(wildcard src/io_wally/codec/*.cpp)
SRCS_M           += $(wildcard src/io_wally/spi/*.cpp)
SRCS_M           += $(wildcard src/io_wally/impl/*.cpp)
SRCS_M           += $(wildcard src/io_wally/dispatch/*.cpp)
SRCS_M           += $(wildcard src/io_wally/app/*.cpp)

EXECSOURCE_M     := $(wildcard src/*.cpp)

# ***********************************************************************
# OBJECTS: main
# *********************************************************************** 

# Build dir for SRCS
BUILD_M          := $(BUILD)/main

# Objects
OBJS_M           := $(patsubst src/%.cpp, $(BUILD_M)/%.o, $(SRCS_M))

EXECOBJ_M        := $(patsubst src/%.cpp, $(BUILD_M)/%.o, $(EXECSOURCE_M))

# Subdirs in build directory need to reflect subdirs in src directory
BUILDDIRS_M      := $(sort $(dir $(OBJS_M)))

# Main executable
EXEC_M           := $(BUILD_M)/mqtt-serverd

# ***********************************************************************
# OBJECTS: release
# ***********************************************************************

# Build dir for SRCS
BUILD_M_RELEASE  := $(BUILD)/release

# Objects
OBJS_M_RELEASE   := $(patsubst src/%.cpp, $(BUILD_M_RELEASE)/%.o, $(SRCS_M))

EXECOBJ_M_RELEASE := $(patsubst src/%.cpp, $(BUILD_M_RELEASE)/%.o, $(EXECSOURCE_M))

# Subdirs in build directory need to reflect subdirs in src directory
BUILDDIRS_M_RELEASE := $(sort $(dir $(OBJS_M_RELEASE)))

# Main executable
EXEC_M_RELEASE   := $(BUILD_M_RELEASE)/mqtt-serverd

# ***********************************************************************
# OBJECTS: debug
# ***********************************************************************

# Build dir for SRCS
BUILD_M_DEBUG    := $(BUILD)/debug

# Objects
OBJS_M_DEBUG     := $(patsubst src/%.cpp, $(BUILD_M_DEBUG)/%.o, $(SRCS_M))

EXECOBJ_M_DEBUG  := $(patsubst src/%.cpp, $(BUILD_M_DEBUG)/%.o, $(EXECSOURCE_M))

# Subdirs in build directory need to reflect subdirs in src directory
BUILDDIRS_M_DEBUG := $(sort $(dir $(OBJS_M_DEBUG)))

# Main executable
EXEC_M_DEBUG     := $(BUILD_M_DEBUG)/mqtt-serverd

# ***********************************************************************
# OBJECTS: sanitize
# ***********************************************************************

# Build dir for SRCS
BUILD_M_SAN      := $(BUILD)/sanitize

# Objects
OBJS_M_SAN       := $(patsubst src/%.cpp, $(BUILD_M_SAN)/%.o, $(SRCS_M))

EXECOBJ_M_SAN    := $(patsubst src/%.cpp, $(BUILD_M_SAN)/%.o, $(EXECSOURCE_M))

# Subdirs in build directory need to reflect subdirs in src directory
BUILDDIRS_M_SAN  := $(sort $(dir $(OBJS_M_SAN)))

# Main executable
EXEC_M_SAN       := $(BUILD_M_SAN)/wally-iod

# --------------------------------------------------------------------------------------------------------------------- 
# Unit tests
# --------------------------------------------------------------------------------------------------------------------- 

# Test SRCS
SRCS_UT           += $(wildcard test/io_wally/*.cpp)
SRCS_UT           += $(wildcard test/io_wally/protocol/*.cpp)
SRCS_UT           += $(wildcard test/io_wally/codec/*.cpp)
SRCS_UT           += $(wildcard test/io_wally/impl/*.cpp)
SRCS_UT           += $(wildcard test/io_wally/app/*.cpp)

EXECSOURCE_UT     := $(wildcard test/*.cpp)

# Build dir for tests
BUILD_UT          := $(BUILD)/test

# Test objects
OBJS_UT           := $(patsubst test/%.cpp, $(BUILD_UT)/%.o, $(SRCS_UT))

EXECOBJ_UT        := $(patsubst test/%.cpp, $(BUILD_UT)/%.o, $(EXECSOURCE_UT))

# Subdirs in build directory need to reflect subdirs in test directory
BUILDDIRS_UT      := $(sort $(dir $(OBJS_UT)))

# Main test executable
EXEC_UT           := $(BUILD_UT)/unit-tests

# --------------------------------------------------------------------------------------------------------------------- 
# Integration tests
# --------------------------------------------------------------------------------------------------------------------- 

# Support libraries
BUILD_SUPPORT    := $(BUILD)/support

# Support: Paho MQTT client
PINC_PAHO        := support/paho/packet
CINC_PAHO        := support/paho/client

PSRCS_PAHO       := $(wildcard support/paho/packet/*.c)
CSRCS_PAHO       := $(wildcard support/paho/client/*.cpp)

PBUILD_PAHO      := $(BUILD_SUPPORT)/paho/packet
CBUILD_PAHO      := $(BUILD_SUPPORT)/paho/client

OBJS_PAHO        := $(patsubst support/paho/packet/%.c, $(PBUILD_PAHO)/%.o, $(PSRCS_PAHO))
OBJS_PAHO        += $(patsubst support/paho/client/%.cpp, $(CBUILD_PAHO)/%.o, $(CSRCS_PAHO))

# Integrationtest SRCS
SRCS_IT          := $(wildcard itest/framework/*.cpp)
SRCS_IT          += $(wildcard itest/io_wally/*.cpp)

EXECSOURCE_IT    := itest/itests_main.cpp

# Build dir for integrationtests
BUILD_IT         := $(BUILD)/itest

# Intgegrationtest objects
OBJS_IT          := $(patsubst itest/%.cpp, $(BUILD_IT)/%.o, $(SRCS_IT))

EXECOBJ_IT       := $(patsubst itest/%.cpp, $(BUILD_IT)/%.o, $(EXECSOURCE_IT))

# Subdirs in build directory need to reflect subdirs in integrationtest directory
BUILDDIRS_IT     := $(sort $(dir $(OBJS_IT)))

# Main integrationtest executable
EXEC_IT          := $(BUILD_IT)/integration-tests

# --------------------------------------------------------------------------------------------------------------------- 
# Snippets: try stuff, analyse bugs etc.
# --------------------------------------------------------------------------------------------------------------------- 

# Snippets SRCS
EXECSOURCE_SNIP  := itest/snippets_main.cpp

EXECOBJ_SNIP     := $(patsubst itest/%.cpp, $(BUILD_IT)/%.o, $(EXECSOURCE_SNIP))

# Main integrationtest executable
EXEC_SNIP        := $(BUILD_IT)/snippets

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
# Tooling
# --------------------------------------------------------------------------------------------------------------------- 

# Where to store scan-build's analysis results
BUILD_SCAN      := $(BUILD)/scan

# All things doxygen
BUILD_DOC       := $(BUILD)/doc

# Clang's compilation database needed for some of its tooling
COMPILATIONDB   := compile_commands.json

# --------------------------------------------------------------------------------------------------------------------- 
# Miscellaneous
# --------------------------------------------------------------------------------------------------------------------- 

# What may be rebuilt
REBUILDABLES    := $(EXEC_M) $(OBJS_M) $(EXEC_UT) $(OBJS_UT) $(EXEC_IT) $(OBJS_IT)

# --------------------------------------------------------------------------------------------------------------------- 
# Compiler configuration: common
# --------------------------------------------------------------------------------------------------------------------- 

# GCC
CXX             := g++
#CXX             := clang++
CC              := gcc

# --------------------------------------------------------------------------------------------------------------------- 
# Compiler configuration: main executable NORMAL
# --------------------------------------------------------------------------------------------------------------------- 

# Standard compiler flags
CXXFLAGS_M      := -std=c++14
CXXFLAGS_M      += -fdiagnostics-color=auto
CXXFLAGS_M      += -MMD # automatically generate dependency rules on each run
CXXFLAGS_M      += -I ./src
CXXFLAGS_M      += -I $(BOOST_ASIO_QE)
CXXFLAGS_M      += -Werror
CXXFLAGS_M      += -Wall
CXXFLAGS_M      += -Wextra
CXXFLAGS_M      += -Wcast-align
CXXFLAGS_M      += -Wformat-nonliteral
CXXFLAGS_M      += -Wformat=2
CXXFLAGS_M      += -Winvalid-pch
CXXFLAGS_M      += -Wmissing-declarations
CXXFLAGS_M      += -Wmissing-format-attribute
CXXFLAGS_M      += -Wmissing-include-dirs
CXXFLAGS_M      += -Wredundant-decls
CXXFLAGS_M      += -Wswitch-default
CXXFLAGS_M      += -Wswitch-enum

# Standard preprocessor flags
CPPFLAGS_M      := -DBOOST_ALL_DYN_LINK
# Needed for clang:
# http://stackoverflow.com/questions/27552028/who-is-failing-boost-clang-or-gcc-issue-with-stdchrono-used-with-boostas
CPPFLAGS_M      += -DBOOST_ASIO_HAS_STD_CHRONO 

# Extra linker flags
LDLIBS_M        := -lboost_system
LDLIBS_M        += -lboost_thread
LDLIBS_M        += -lboost_log
LDLIBS_M        += -lboost_log_setup
LDLIBS_M        += -lboost_program_options
LDLIBS_M        += -lpthread

# --------------------------------------------------------------------------------------------------------------------- 
# Compiler configuration: main executable RELEASE
# --------------------------------------------------------------------------------------------------------------------- 

CXXFLAGS_REL    := $(CXXFLAGS_M)
CXXFLAGS_REL    += -O3

# --------------------------------------------------------------------------------------------------------------------- 
# Compiler configuration: main executable DEBUG (currently not working since we miss debug info for boost
# program_options)
# --------------------------------------------------------------------------------------------------------------------- 

CXXFLAGS_DEBUG  := $(CXXFLAGS_M)
CXXFLAGS_DEBUG  += -O0 -g
CXXFLAGS_DEBUG  += -D_GLIBCXX_DEBUG
CXXFLAGS_DEBUG  += -DBOOST_ASIO_ENABLE_HANDLER_TRACKING

# --------------------------------------------------------------------------------------------------------------------- 
# Compiler configuration: main executable SANITIZE
# --------------------------------------------------------------------------------------------------------------------- 

# Compiler/linker flags for checking programmer's sanity: copied from seastar's build
# See: http://btorpey.github.io/blog/2014/03/27/using-clangs-address-sanitizer/
CXXFLAGS_SAN     := $(CXXFLAGS_M)
CXXFLAGS_SAN     += -O0
CXXFLAGS_SAN     += -fsanitize=address
CXXFLAGS_SAN     += -fsanitize=leak
CXXFLAGS_SAN     += -fsanitize=undefined
CXXFLAGS_SAN     += -fno-omit-frame-pointer

# Sanitizer linker flags
LDLIBS_SAN       := $(LDLIBS_M)
LDLIBS_SAN       += -fsanitize=address
LDLIBS_SAN       += -fsanitize=leak
LDLIBS_SAN       += -fsanitize=undefined

# --------------------------------------------------------------------------------------------------------------------- 
# Compiler configuration: unit tests
# --------------------------------------------------------------------------------------------------------------------- 

# Test compiler flags
CXXFLAGS_UT     := $(CXXFLAGS_M)
CXXFLAGS_UT     += -O0 -g
CXXFLAGS_UT     += -I ./test
CXXFLAGS_UT     := $(filter-out -Wswitch-default, $(CXXFLAGS_UT))
CXXFLAGS_UT     := $(filter-out -Wswitch-enum, $(CXXFLAGS_UT))
CXXFLAGS_UT     += -fsanitize=address
CXXFLAGS_UT     += -fsanitize=leak
CXXFLAGS_UT     += -fsanitize=undefined
CXXFLAGS_UT     += -fno-omit-frame-pointer

# Test linker flags
LDLIBS_UT       := $(LDLIBS_M)
LDLIBS_UT       += -fsanitize=address
LDLIBS_UT       += -fsanitize=leak
LDLIBS_UT       += -fsanitize=undefined

# --------------------------------------------------------------------------------------------------------------------- 
# Compiler configuration: integration tests
# --------------------------------------------------------------------------------------------------------------------- 

# Integrationtest compiler flags
CXXFLAGS_IT     := $(CXXFLAGS_M)
CXXFLAGS_IT     += -O0 -g
CXXFLAGS_IT     += -I ./itest
CXXFLAGS_IT     += -I $(PINC_PAHO)
CXXFLAGS_IT     += -I $(CINC_PAHO)
CXXFLAGS_IT     := $(filter-out -Wswitch-default, $(CXXFLAGS_IT))
CXXFLAGS_IT     := $(filter-out -Wswitch-enum, $(CXXFLAGS_IT))
CXXFLAGS_IT     += -fsanitize=address
CXXFLAGS_IT     += -fsanitize=leak
CXXFLAGS_IT     += -fsanitize=undefined
CXXFLAGS_IT     += -fno-omit-frame-pointer

# Integrationtest linker flags
LDLIBS_IT       := $(LDLIBS_M)
LDLIBS_IT       += -fsanitize=address
LDLIBS_IT       += -fsanitize=leak
LDLIBS_IT       += -fsanitize=undefined

#######################################################################################################################
# Rules
#######################################################################################################################

# --------------------------------------------------------------------------------------------------------------------- 
# Build main executable NORMAL
# --------------------------------------------------------------------------------------------------------------------- 

main                : $(EXEC_M)                              | $(BUILDDIRS_M)

$(BUILDDIRS_M)      :
	@mkdir -p $@

$(EXEC_M)           : $(OBJS_M) $(EXECOBJ_M)                 | $(BUILDDIRS_M)
	$(CXX) $(LDLIBS_M) -o $@ $^

$(BUILD_M)/%.o      : src/%.cpp                              | $(BUILDDIRS_M)
	$(CXX) $(CPPFLAGS_M) $(CXXFLAGS_M) -o $@ -c $<

# --------------------------------------------------------------------------------------------------------------------- 
# Build main executable RELEASE
# --------------------------------------------------------------------------------------------------------------------- 

release             : $(EXEC_M_RELEASE)                      | $(BUILDDIRS_M_RELEASE)

$(BUILDDIRS_M_RELEASE) :
	@mkdir -p $@

$(EXEC_M_RELEASE)   : $(OBJS_M_RELEASE) $(EXECOBJ_M_RELEASE) | $(BUILDDIRS_M_RELEASE)
	$(CXX) $(LDLIBS_M) -o $@ $^

$(BUILD_M_RELEASE)/%.o : src/%.cpp                           | $(BUILDDIRS_M_RELEASE)
	$(CXX) $(CPPFLAGS_M) $(CXXFLAGS_REL) -o $@ -c $<

# --------------------------------------------------------------------------------------------------------------------- 
# Build main executable DEBUG
# --------------------------------------------------------------------------------------------------------------------- 

debug               : $(EXEC_M_DEBUG)                        | $(BUILDDIRS_M_DEBUG)

$(BUILDDIRS_M_DEBUG) :
	@mkdir -p $@

$(EXEC_M_DEBUG)     : $(OBJS_M_DEBUG) $(EXECOBJ_M_DEBUG)     | $(BUILDDIRS_M_DEBUG)
	$(CXX) $(LDLIBS_M) -o $@ $^

$(BUILD_M_DEBUG)/%.o : src/%.cpp                             | $(BUILDDIRS_M_DEBUG)
	$(CXX) $(CPPFLAGS_M) $(CXXFLAGS_DEBUG) -o $@ -c $<

# --------------------------------------------------------------------------------------------------------------------- 
# Build main executable SANITIZE
# --------------------------------------------------------------------------------------------------------------------- 

sanitize           : $(EXEC_M_SAN)                           | $(BUILDDIRS_M_SAN)

$(BUILDDIRS_M_SAN) :
	@mkdir -p $@

$(EXEC_M_SAN)      : $(OBJS_M_SAN) $(EXECOBJ_M_SAN)          | $(BUILDDIRS_M_SAN)
	$(CXX) $(LDLIBS_SAN) -o $@ $^

$(BUILD_M_SAN)/%.o : src/%.cpp                               | $(BUILDDIRS_M_SAN)
	$(CXX) $(CPPFLAGS_M) $(CXXFLAGS_SAN) -o $@ -c $<

# --------------------------------------------------------------------------------------------------------------------- 
# Build/run unit tests
# --------------------------------------------------------------------------------------------------------------------- 

.PHONY              : test
test                : test-compile
	@./$(EXEC_UT)

.PHONY              : test-success
test-success        : test-compile
	@./$(EXEC_UT) --success --durations yes

test-compile        : $(EXEC_UT)                             | $(BUILDDIRS_UT)

$(BUILDDIRS_UT)     :
	@mkdir -p $@

$(EXEC_UT)          : $(OBJS_M_SAN) $(OBJS_UT) $(EXECOBJ_UT) | $(BUILDDIRS_UT)
	$(CXX) $(LDLIBS_UT) -o $@ $^

$(BUILD_UT)/%.o     : test/%.cpp                             | $(BUILDDIRS_UT)
	$(CXX) $(CXXFLAGS_UT) -o $@ -c $<

# --------------------------------------------------------------------------------------------------------------------- 
# Build Paho MQTT client to support integration tests
# --------------------------------------------------------------------------------------------------------------------- 

$(PBUILD_PAHO)      :
	@mkdir -p $@

$(PBUILD_PAHO)/%.o  : support/paho/packet/%.c                | $(PBUILD_PAHO)
	$(CC) -o $@ -c $<

$(CBUILD_PAHO)      :
	@mkdir -p $@

$(CBUILD_PAHO)/%.o  : support/paho/client/%.cpp              | $(CBUILD_PAHO)
	$(CXX) -o $@ -c $<

# --------------------------------------------------------------------------------------------------------------------- 
# Build/run integration tests
# --------------------------------------------------------------------------------------------------------------------- 

.PHONY             : itest
itest              : itest-compile
	@./$(EXEC_IT) --success --durations yes

itest-compile      : $(EXEC_IT)                              | $(BUILDDIRS_IT)

$(BUILDDIRS_IT)    :
	@mkdir -p $@

$(EXEC_IT)         : $(OBJS_M_SAN) $(OBJS_IT) $(EXECOBJ_IT) $(OBJS_PAHO)    | $(BUILDDIRS_IT)
	$(CXX) $(LDLIBS_IT) -o $@ $^

$(BUILD_IT)/%.o    : itest/%.cpp                             | $(BUILDDIRS_IT)
	$(CXX) $(CPPFLAGS_M) $(CXXFLAGS_IT) -o $@ -c $<

# --------------------------------------------------------------------------------------------------------------------- 
# Build snippets: try stuff, analyse bugs etc
# --------------------------------------------------------------------------------------------------------------------- 

.PHONY             : snippets 
snippets           : snippets-compile
	@./$(EXEC_SNIP)

.PHONY             : snippets-compile
snippets-compile   : $(EXEC_SNIP)                            | $(BUILDDIRS_IT)

$(EXEC_SNIP)       : $(OBJS_M) $(OBJS_IT) $(EXECOBJ_SNIP)    | $(BUILDDIRS_IT)
	$(CXX) $(LDLIBS_IT) -o $@ $^

# --------------------------------------------------------------------------------------------------------------------- 
# Clean up the mess
# --------------------------------------------------------------------------------------------------------------------- 

.PHONY              : clean
clean               :
	@rm -rf $(BUILD)

# --------------------------------------------------------------------------------------------------------------------- 
# Generate/publish documentation
# --------------------------------------------------------------------------------------------------------------------- 

.PHONY              : doc
doc                 : $(SRCS_M) $(EXECSOURCE_M)
	@rm -rf $(BUILD_DOC)
	@doxygen ./.doxygen.cfg

.PHONY              : doc-publish
doc-publish         : doc
	@pushd $(BUILD_DOC); python -mSimpleHTTPServer 8000; popd

# --------------------------------------------------------------------------------------------------------------------- 
# Tooling
# --------------------------------------------------------------------------------------------------------------------- 

compilation-db      : $(COMPILATIONDB)

$(COMPILAIONDB)     : $(SRCS_M) $(EXECSOURCE_M) $(SRCS_UT) $(EXECSOURCE_UT) $(SRCS_IT) $(EXECSOURCE_IT)
	@bear $(AKE_M) clean main test-compile itest-compile

.PHONY              : macroexpand
macroexpand         : $(SRCS_M) $(EXECSOURCE_M)
	$(CXX) $(CXXFLAGS_M) -E $(SRCS_M) | source-highlight --failsafe --src-lang=cc -f esc --style-file=esc.style 

# Running clang-check
.PHONY              : check-main
check-main          : compilation-db
check-main          : $(SRCS_M) $(EXECSOURCE_M)
	@clang-check $(SRCS_M) $(EXECSOURCE_M)

.PHONY              : check-test
check-test          : compilation-db
check-test          : $(SRCS_UT) $(EXECSOURCE_UT)
	@clang-check $(SRCS_UT) $(EXECSOURCE_UT)

.PHONY              : check-itest
check-itest         : compilation-db
check-itest         : $(SRCS_IT) $(EXECSOURCE_IT)
	@clang-check $(SRCS_IT) $(EXECSOURCE_IT)

.PHONY              : check
check               : check-main check-test 

# Running scan-build
$(BUILD_SCAN)       :
	@mkdir -p $@

.PHONY              : scan-main
scan-main           : $(BUILD_SCAN)
	@scan-build -o $(BUILD_SCAN) -analyze-headers --status-bugs $(MAKE) clean main

.PHONY              : modernize
modernize           : $(SRCS_M) $(EXECSOURCE_M) $(COMPILATIONDB)
	@clang-modernize -final-syntax-check -summary -format -style=file -include=src/ -p $(COMPILATIONDB)

.PHONY              : format-main
format-main         : $(SRCS_M) $(EXECSOURCE_M)
	@clang-format -i -style=file $(SRCS_M) $(EXECSOURCE_M)

.PHONY              : format-test
format-test         : $(SRCS_UT) $(EXECSOURCE_UT)
	@clang-format -i -style=file $(SRCS_UT) $(EXECSOURCE_UT)

.PHONY              : format-itest
format-itest        : $(SRCS_IT) $(EXECSOURCE_IT)
	@clang-format -i -style=file $(SRCS_IT) $(EXECSOURCE_IT)

.PHONY              : format
format              : format-main format-test format-itest

.PHONY              : tags
tags                : $(SRCS_M) $(EXECSOURCE_M) $(SRCS_UT) $(EXECSOURCE_UT) $(SRCS_IT) $(EXECSOURCE_IT)
	@ctags -R -f ./.tags ./src ./test ./itest

# --------------------------------------------------------------------------------------------------------------------- 
# Prepare a commit: run (almost) everything
# --------------------------------------------------------------------------------------------------------------------- 

.PHONY              : prepare-commit
prepare-commit      : clean
prepare-commit      : format
prepare-commit      : sanitize
prepare-commit      : test 
#prepare-commit      : itest # Sadly, we have to deactivate this for now: integration tests crash when stopping app
prepare-commit      : check
prepare-commit      : scan-main

# --------------------------------------------------------------------------------------------------------------------- 
# Helpers
# --------------------------------------------------------------------------------------------------------------------- 

# Run server with some convenient default settings
.PHONY              : run-server
run-server          : $(EXEC_M)
	@$(EXEC_M) --log-file .testlog --log-file-level trace --log-console --log-console-level trace --conn-timeout 1000000

#######################################################################################################################
# Dependency rules: http://stackoverflow.com/questions/8025766/makefile-auto-dependency-generation
#######################################################################################################################

-include            $(OBJS_M:.o=.d)
-include            $(EXECOBJ_M:.o=.d)
-include            $(OBJS_UT:.o=.d)
-include            $(EXECOBJ_UT:.o=.d)
-include            $(OBJS_IT:.o=.d)
-include            $(EXECOBJ_IT:.o=.d)
