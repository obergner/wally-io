# --------------------------------------------------------------------------------------------------------------------- 
# Compiler configuration/main: config = run
# --------------------------------------------------------------------------------------------------------------------- 

# Standard compiler flags
CXXFLAGS_RUN              := -std=c++17
CXXFLAGS_RUN              += -c # Only compile
CXXFLAGS_RUN              += -fdiagnostics-color=auto
CXXFLAGS_RUN              += -MMD # automatically generate dependency rules on each run
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

# Standard linker flags, if any
LDFLAGS_RUN               :=

# Standard link libraries
# See: https://stackoverflow.com/questions/23250863/difference-between-pthread-and-lpthread-while-compiling
LDLIBS_RUN                += -pthread

# --------------------------------------------------------------------------------------------------------------------- 
# Compiler configuration/main: config = release
# --------------------------------------------------------------------------------------------------------------------- 

CXXFLAGS_REL              := $(CXXFLAGS_RUN)
CXXFLAGS_REL              += -O3

# Release build preprocessor flags
CPPFLAGS_REL              := $(CPPFLAGS_RUN)

# Release linker flags: link statically
LDFLAGS_REL               := $(LDFLAGS_RUN)
LDFLAGS_REL               += -static
LDFLAGS_REL               += -static-libstdc++

# Release linker flags
LDLIBS_REL                := $(LDLIBS_RUN)

# --------------------------------------------------------------------------------------------------------------------- 
# Compiler configuration/main: config = debug
# --------------------------------------------------------------------------------------------------------------------- 

CXXFLAGS_DEBUG            := $(CXXFLAGS_RUN)
CXXFLAGS_DEBUG            += -O0
CXXFLAGS_DEBUG            += -g
#CXXFLAGS_DEBUG            += -DASIO_ENABLE_HANDLER_TRACKING
CXXFLAGS_DEBUG            += -D_GLIBCXX_DEBUG
CXXFLAGS_DEBUG            += -D_GLIBCXX_DEBUG_PEDANTIC
CXXFLAGS_DEBUG            += -D_GLIBCXX_ASSERTIONS
# I want to use this flag but currently enabling it causes a very strange error in Asio's mutable_buffer ctor ONLY
# when sending a previously retained PUBLISH to a newly connected client.
# TODO: fix issue with -DASIO_ENABLE_BUFFER_DEBUGGING
CXXFLAGS_DEBUG            += -DASIO_DISABLE_BUFFER_DEBUGGING

# Debug build preprocessor flags
CPPFLAGS_DEBUG            := $(CPPFLAGS_RUN)

# Debug linker flags, if any
LDFLAGS_DEBUG             := $(LDFLAGS_RUN)

# Debug link libraries
LDLIBS_DEBUG              := $(LDLIBS_RUN)

# --------------------------------------------------------------------------------------------------------------------- 
# Compiler configuration/main: config = asan
# --------------------------------------------------------------------------------------------------------------------- 

# Compiler/linker flags for checking programmer's sanity: copied from seastar's build
# See: http://btorpey.github.io/blog/2014/03/27/using-clangs-address-sanitizer/
CXXFLAGS_ASAN             := $(CXXFLAGS_RUN)
CXXFLAGS_ASAN             += -O0
CXXFLAGS_ASAN             += -g # Needed by g++ to support line numbers in asan reports
CXXFLAGS_ASAN             += -D_GLIBCXX_DEBUG
CXXFLAGS_ASAN             += -D_GLIBCXX_DEBUG_PEDANTIC
CXXFLAGS_ASAN             += -D_GLIBCXX_ASSERTIONS
# I want to use this flag but currently enabling it causes a very strange error in Asio's mutable_buffer ctor ONLY
# when sending a previously retained PUBLISH to a newly connected client.
# TODO: fix issue with -DASIO_ENABLE_BUFFER_DEBUGGING
CXXFLAGS_ASAN             += -DASIO_DISABLE_BUFFER_DEBUGGING
CXXFLAGS_ASAN             += -fsanitize=address
CXXFLAGS_ASAN             += -fsanitize=leak
CXXFLAGS_ASAN             += -fsanitize=undefined
CXXFLAGS_ASAN             += -fno-omit-frame-pointer

# Asan build preprocessor flags
CPPFLAGS_ASAN             := $(CPPFLAGS_RUN)

# Asan linker flags
LDFLAGS_ASAN              := $(LDFLAGS_RUN)
LDFLAGS_ASAN              += -fsanitize=address
LDFLAGS_ASAN              += -fsanitize=leak
LDFLAGS_ASAN              += -fsanitize=undefined

# Asan link libraries
LDLIBS_ASAN               := $(LDLIBS_RUN)

# --------------------------------------------------------------------------------------------------------------------- 
# Compiler configuration/main: config = tsan
# --------------------------------------------------------------------------------------------------------------------- 

CXXFLAGS_TSAN             := $(CXXFLAGS_RUN)
CXXFLAGS_TSAN             += -O2
CXXFLAGS_TSAN             += -g # Needed by g++ to support line numbers in asan reports
CXXFLAGS_TSAN             += -fsanitize=thread
CXXFLAGS_TSAN             += -fno-omit-frame-pointer

# Asan build preprocessor flags
CPPFLAGS_TSAN             := $(CPPFLAGS_RUN)

# Asan linker flags
LDFLAGS_TSAN              := $(LDFLAGS_RUN)
LDFLAGS_TSAN              += -fsanitize=thread

# Asan link libraries
LDLIBS_TSAN               := $(LDLIBS_RUN)

# --------------------------------------------------------------------------------------------------------------------- 
# Compiler configuration/test: config = run
# --------------------------------------------------------------------------------------------------------------------- 

# Test compiler flags
CXXFLAGS_UT_RUN           := $(CXXFLAGS_RUN)
CXXFLAGS_UT_RUN           := $(filter-out -Wswitch-default, $(CXXFLAGS_UT_RUN))
CXXFLAGS_UT_RUN           := $(filter-out -Wswitch-enum, $(CXXFLAGS_UT_RUN))

# Test preprocessor flags
CPPFLAGS_UT_RUN           := $(CPPFLAGS_RUN)

# Test linker flags, if any
LDFLAGS_UT_RUN            :=

# Test link libraries
LDLIBS_UT_RUN             := $(LDLIBS_RUN)

# --------------------------------------------------------------------------------------------------------------------- 
# Compiler configuration/test: config = release
# --------------------------------------------------------------------------------------------------------------------- 

# Test compiler flags
CXXFLAGS_UT_REL           := $(CXXFLAGS_UT_RUN)

# Test preprocessor flags
CPPFLAGS_UT_REL           := $(CPPFLAGS_UT_RUN)

# Test linker flags, if any
LDFLAGS_UT_REL            := $(LDFLAGS_UT_RUN)

# Test link libraries
LDLIBS_UT_REL             := $(LDLIBS_UT_RUN)

# --------------------------------------------------------------------------------------------------------------------- 
# Compiler configuration/test: config = debug
# --------------------------------------------------------------------------------------------------------------------- 

# Test compiler flags
CXXFLAGS_UT_DEBUG         := $(CXXFLAGS_UT_RUN)
CXXFLAGS_UT_DEBUG         += -O0
CXXFLAGS_UT_DEBUG         += -g # Needed by g++ to support line numbers in asan reports
CXXFLAGS_UT_DEBUG         += -D_GLIBCXX_DEBUG
CXXFLAGS_UT_DEBUG         += -D_GLIBCXX_DEBUG_PEDANTIC
CXXFLAGS_UT_DEBUG         += -D_GLIBCXX_ASSERTIONS
# I want to use this flag but currently enabling it causes a very strange error in Asio's mutable_buffer ctor ONLY
# when sending a previously retained PUBLISH to a newly connected client.
# TODO: fix issue with -DASIO_ENABLE_BUFFER_DEBUGGING
CXXFLAGS_UT_DEBUG         += -DASIO_DISABLE_BUFFER_DEBUGGING

# Test preprocessor flags
CPPFLAGS_UT_DEBUG         := $(CPPFLAGS_UT_RUN)

# Test linker flags, if any
LDFLAGS_UT_DEBUG          := $(LDFLAGS_UT_RUN)

# Test link libraries
LDLIBS_UT_DEBUG           := $(LDLIBS_UT_RUN)

# --------------------------------------------------------------------------------------------------------------------- 
# Compiler configuration/test: config = asan
# --------------------------------------------------------------------------------------------------------------------- 

# Test compiler flags
CXXFLAGS_UT_ASAN          := $(CXXFLAGS_UT_RUN)
CXXFLAGS_UT_ASAN          += -O0
CXXFLAGS_UT_ASAN          += -g # Needed by g++ to support line numbers in asan reports
CXXFLAGS_UT_ASAN          += -D_GLIBCXX_DEBUG
CXXFLAGS_UT_ASAN          += -D_GLIBCXX_DEBUG_PEDANTIC
CXXFLAGS_UT_ASAN          += -D_GLIBCXX_ASSERTIONS
# I want to use this flag but currently enabling it causes a very strange error in Asio's mutable_buffer ctor ONLY
# when sending a previously retained PUBLISH to a newly connected client.
# TODO: fix issue with -DASIO_ENABLE_BUFFER_DEBUGGING
CXXFLAGS_UT_ASAN          += -DASIO_DISABLE_BUFFER_DEBUGGING
CXXFLAGS_UT_ASAN          += -fsanitize=address
CXXFLAGS_UT_ASAN          += -fsanitize=leak
CXXFLAGS_UT_ASAN          += -fsanitize=undefined
CXXFLAGS_UT_ASAN          += -fno-omit-frame-pointer

# Test preprocessor flags
CPPFLAGS_UT_ASAN          := $(CPPFLAGS_UT_RUN)

# Test linker flags, if any
LDFLAGS_UT_ASAN           := $(LDFLAGS_UT_RUN)
LDFLAGS_UT_ASAN           += -fsanitize=address
LDFLAGS_UT_ASAN           += -fsanitize=leak
LDFLAGS_UT_ASAN           += -fsanitize=undefined

# Test link libraries
LDLIBS_UT_ASAN            := $(LDLIBS_UT_RUN)

# --------------------------------------------------------------------------------------------------------------------- 
# Compiler configuration/test: config = tsan
# --------------------------------------------------------------------------------------------------------------------- 

# Test compiler flags
CXXFLAGS_UT_TSAN          := $(CXXFLAGS_UT_RUN)
CXXFLAGS_UT_TSAN          += -O2
CXXFLAGS_UT_TSAN          += -g # Needed by g++ to support line numbers in asan reports
CXXFLAGS_UT_TSAN          += -fsanitize=thread
CXXFLAGS_UT_TSAN          += -fno-omit-frame-pointer

# Test preprocessor flags
CPPFLAGS_UT_TSAN          := $(CPPFLAGS_UT_RUN)

# Test linker flags, if any
LDFLAGS_UT_TSAN           := $(LDFLAGS_UT_RUN)
LDFLAGS_UT_TSAN           += -fsanitize=thread

# Test link libraries
LDLIBS_UT_TSAN            := $(LDLIBS_UT_RUN)

