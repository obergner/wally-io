#######################################################################################################################
# Download and build support libraries that we do not have native (RPM) packages for
#######################################################################################################################

# -------------------------------------------------------------------------------- 
# Common
# -------------------------------------------------------------------------------- 

# http://stackoverflow.com/questions/18136918/how-to-get-current-directory-of-your-makefile
CWD                             := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

# -------------------------------------------------------------------------------- 
# Asio header only
# -------------------------------------------------------------------------------- 

ASIO_EXT_DIR                    := $(CWD)/asio
ASIO_EXT_INC                    := $(ASIO_EXT_DIR)

ASIO_EXT_SRCS                   := $(wildcard $(ASIO_EXT_INC)/asio.hpp)
ASIO_EXT_SRCS                   := $(wildcard $(ASIO_EXT_INC)/asio/*.hpp)
ASIO_EXT_SRCS                   += $(wildcard $(ASIO_EXT_INC)/asio/detail/*.hpp)
ASIO_EXT_SRCS                   += $(wildcard $(ASIO_EXT_INC)/asio/detail/impl/*.hpp)
ASIO_EXT_SRCS                   += $(wildcard $(ASIO_EXT_INC)/asio/detail/impl/*.ipp)
ASIO_EXT_SRCS                   += $(wildcard $(ASIO_EXT_INC)/asio/generic/*.hpp)
ASIO_EXT_SRCS                   += $(wildcard $(ASIO_EXT_INC)/asio/generic/detail/*.hpp)
ASIO_EXT_SRCS                   += $(wildcard $(ASIO_EXT_INC)/asio/generic/detail/impl/*.ipp)
ASIO_EXT_SRCS                   += $(wildcard $(ASIO_EXT_INC)/asio/impl/*.hpp)
ASIO_EXT_SRCS                   += $(wildcard $(ASIO_EXT_INC)/asio/impl/*.ipp)
ASIO_EXT_SRCS                   += $(wildcard $(ASIO_EXT_INC)/asio/impl/*.cpp)
ASIO_EXT_SRCS                   += $(wildcard $(ASIO_EXT_INC)/asio/ip/*.hpp)
ASIO_EXT_SRCS                   += $(wildcard $(ASIO_EXT_INC)/asio/ip/detail/*.hpp)
ASIO_EXT_SRCS                   += $(wildcard $(ASIO_EXT_INC)/asio/ip/detail/impl/*.hpp)
ASIO_EXT_SRCS                   += $(wildcard $(ASIO_EXT_INC)/asio/ip/detail/impl/*.ipp)
ASIO_EXT_SRCS                   += $(wildcard $(ASIO_EXT_INC)/asio/ip/impl/*.ipp)
ASIO_EXT_SRCS                   += $(wildcard $(ASIO_EXT_INC)/asio/local/*.hpp)
ASIO_EXT_SRCS                   += $(wildcard $(ASIO_EXT_INC)/asio/local/detail/*.hpp)
ASIO_EXT_SRCS                   += $(wildcard $(ASIO_EXT_INC)/asio/local/detail/impl/*.ipp)
ASIO_EXT_SRCS                   += $(wildcard $(ASIO_EXT_INC)/asio/posix/*.hpp)
ASIO_EXT_SRCS                   += $(wildcard $(ASIO_EXT_INC)/asio/ssl/*.hpp)
ASIO_EXT_SRCS                   += $(wildcard $(ASIO_EXT_INC)/asio/ssl/detail/*.hpp)
ASIO_EXT_SRCS                   += $(wildcard $(ASIO_EXT_INC)/asio/ssl/detail/impl/*.ipp)
ASIO_EXT_SRCS                   += $(wildcard $(ASIO_EXT_INC)/asio/ssl/impl/*.hpp)
ASIO_EXT_SRCS                   += $(wildcard $(ASIO_EXT_INC)/asio/ssl/impl/*.ipp)
ASIO_EXT_SRCS                   += $(wildcard $(ASIO_EXT_INC)/asio/ssl/old/*.hpp)
ASIO_EXT_SRCS                   += $(wildcard $(ASIO_EXT_INC)/asio/ssl/old/detail/*.hpp)
ASIO_EXT_SRCS                   += $(wildcard $(ASIO_EXT_INC)/asio/windows/*.hpp)

# -------------------------------------------------------------------------------- 
# spdlog header only
# -------------------------------------------------------------------------------- 

SPDLOG_EXT_DIR                  := $(CWD)/spdlog
SPDLOG_EXT_INC                  := $(SPDLOG_EXT_DIR)

SPDLOG_EXT_SRCS                 := $(wildcard $(SPDLOG_EXT_INC)/spdlog/*.h)
SPDLOG_EXT_SRCS                 += $(wildcard $(SPDLOG_EXT_INC)/spdlog/details/*.h)
SPDLOG_EXT_SRCS                 += $(wildcard $(SPDLOG_EXT_INC)/spdlog/fmt/*.h)
SPDLOG_EXT_SRCS                 += $(wildcard $(SPDLOG_EXT_INC)/spdlog/fmt/bundled/*.h)
SPDLOG_EXT_SRCS                 += $(wildcard $(SPDLOG_EXT_INC)/spdlog/fmt/bundled/*.cc)
SPDLOG_EXT_SRCS                 += $(wildcard $(SPDLOG_EXT_INC)/spdlog/sinks/*.h)

