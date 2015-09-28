#pragma once

#define BOOST_ALL_DYN_LINK 1

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>

namespace keywords = boost::log::keywords;
namespace lvl = boost::log::trivial;
