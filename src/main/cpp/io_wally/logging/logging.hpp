#pragma once

#include <algorithm>
#include <array>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <cxxopts.hpp>

#include <spdlog/spdlog.h>

namespace io_wally
{
    namespace logging
    {
        /// \brief Factory for \c spdlog::logger instances
        ///
        class logger_factory final
        {
           public:  // static
            static logger_factory create( const cxxopts::ParseResult& config );

            static logger_factory disabled( );

            logger_factory( const std::string log_pattern,
                            spdlog::level::level_enum log_level,
                            const std::vector<spdlog::sink_ptr> sinks );

           public:
            std::unique_ptr<spdlog::logger> logger( const std::string& logger_name ) const;

           private:  // static
            static std::unique_ptr<logger_factory> instance_;

           private:
            const std::string log_pattern_;
            spdlog::level::level_enum log_level_;
            const std::vector<spdlog::sink_ptr> sinks_;
        };  // class logger_factory
    }       // namespace loggers
}  // namespace io_wally
