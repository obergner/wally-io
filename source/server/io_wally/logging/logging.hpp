#pragma once

#include <algorithm>
#include <array>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <cxxopts.hpp>

#include <spdlog/spdlog.h>

namespace io_wally::logging
{
    /// \brief Factory for \c spdlog::logger instances
    ///
    class logger_factory final
    {
       public:  // static
        static auto create( const cxxopts::ParseResult& config ) -> logger_factory;

        static auto disabled( ) -> logger_factory;

        logger_factory( std::string log_pattern,
                        spdlog::level::level_enum log_level,
                        std::vector<spdlog::sink_ptr> sinks );

       public:
        [[nodiscard]] auto logger( const std::string& logger_name ) const -> std::unique_ptr<spdlog::logger>;

       private:  // static
        static std::unique_ptr<logger_factory> instance_;

       private:
        const std::string log_pattern_;
        spdlog::level::level_enum log_level_;
        const std::vector<spdlog::sink_ptr> sinks_;
    };  // class logger_factory
}  // namespace io_wally::logging
