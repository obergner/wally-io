#######################################################################################################################
# Download and build support libraries that we do not have native (RPM) packages for
#######################################################################################################################

# -------------------------------------------------------------------------------- 
# Common
# -------------------------------------------------------------------------------- 

# http://stackoverflow.com/questions/18136918/how-to-get-current-directory-of-your-makefile
CWD                             := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

# -------------------------------------------------------------------------------- 
# Asio header only: https://think-async.com/
# -------------------------------------------------------------------------------- 

ASIO_EXT_DIR                    := $(CWD)/asio
ASIO_EXT_INC                    := $(ASIO_EXT_DIR)/include

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
# spdlog header only: https://github.com/gabime/spdlog
# -------------------------------------------------------------------------------- 

SPDLOG_EXT_DIR                  := $(CWD)/spdlog
SPDLOG_EXT_INC                  := $(SPDLOG_EXT_DIR)/include

SPDLOG_EXT_SRCS                 := $(wildcard $(SPDLOG_EXT_INC)/spdlog/*.h)
SPDLOG_EXT_SRCS                 += $(wildcard $(SPDLOG_EXT_INC)/spdlog/details/*.h)
SPDLOG_EXT_SRCS                 += $(wildcard $(SPDLOG_EXT_INC)/spdlog/fmt/*.h)
SPDLOG_EXT_SRCS                 += $(wildcard $(SPDLOG_EXT_INC)/spdlog/fmt/bundled/*.h)
SPDLOG_EXT_SRCS                 += $(wildcard $(SPDLOG_EXT_INC)/spdlog/fmt/bundled/*.cc)
SPDLOG_EXT_SRCS                 += $(wildcard $(SPDLOG_EXT_INC)/spdlog/sinks/*.h)

# -------------------------------------------------------------------------------- 
# cxxopts header only: https://github.com/jarro2783/cxxopts
# -------------------------------------------------------------------------------- 

CXXOPTS_EXT_DIR                 := $(CWD)/cxxopts
CXXOPTS_EXT_INC                 := $(CXXOPTS_EXT_DIR)/include

CXXOPTS_EXT_SRCS                := $(wildcard $(CXXOPTS_EXT_INC)/*.hpp)

# -------------------------------------------------------------------------------- 
# Catch2 header only: https://github.com/catchorg/Catch2
# -------------------------------------------------------------------------------- 

CATCH2_EXT_DIR                  := $(CWD)/catch2
CATCH2_EXT_INC                  := $(CATCH2_EXT_DIR)/include

CATCH2_EXT_SRCS                 := $(wildcard $(CATCH2_EXT_INC)/*.hpp)

CATCH2_DL_URL                   := https://github.com/catchorg/Catch2/releases/latest/download/catch.hpp

.PHONY                          : upgrade-catch-hpp
upgrade-catch-hpp               :
	@mv $(CATCH2_EXT_INC)/catch.hpp $(CATCH2_EXT_INC)/catch.hpp.backup; \
		curl --location --progress-bar --output $(CATCH2_EXT_INC)/catch.hpp $(CATCH2_DL_URL);

.PHONY                          : revert-catch-hpp
revert-catch-hpp                :
	@mv $(CATCH2_EXT_INC)/catch.hpp.backup $(CATCH2_EXT_INC)/catch.hpp

# -------------------------------------------------------------------------------- 
# All external sources combined
# -------------------------------------------------------------------------------- 

EXT_SRCS                        := $(ASIO_EXT_SRCS)
EXT_SRCS                        += $(SPDLOG_EXT_SRCS)
EXT_SRCS                        += $(CXXOPTS_EXT_SRCS)
EXT_SRCS                        += $(CATCH2_EXT_SRCS)

