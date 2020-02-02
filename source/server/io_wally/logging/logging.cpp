#include "io_wally/logging/logging.hpp"

#include <algorithm>
#include <exception>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <cxxopts.hpp>

#include <spdlog/sinks/null_sink.h>
#include <spdlog/spdlog.h>

#include "io_wally/context.hpp"
#include "io_wally/defaults.hpp"
#include "io_wally/logging_support.hpp"

namespace io_wally::logging
{
    namespace
    {
        const std::map<const std::string, spdlog::level::level_enum> LEVELS_BY_NAME = {
            {"trace", spdlog::level::level_enum::trace}, {"debug", spdlog::level::level_enum::debug},
            {"info", spdlog::level::level_enum::info},   {"warn", spdlog::level::level_enum::warn},
            {"err", spdlog::level::level_enum::err},     {"critical", spdlog::level::level_enum::critical}};

        auto is_supported_log_level( const std::string& log_level ) -> bool
        {
            return LEVELS_BY_NAME.find( log_level ) != LEVELS_BY_NAME.end( );
        }

        auto safe_log_level_filter( const std::string& log_level, spdlog::level::level_enum default_level )
            -> spdlog::level::level_enum
        {
            if ( !is_supported_log_level( log_level ) )
            {
                std::cerr << "ERROR: log level \"" << log_level
                          << "\" is not supported and will be replaced with default log level \"" << default_level
                          << "\"" << std::endl
                          << "HINT:  supported log levels are: (trace|debug|info|warn|err|critical)" << std::endl;

                return default_level;
            }

            return LEVELS_BY_NAME.at( log_level );
        }
    }  // namespace
    // --------------------------------------------------------------------------------------------------
    // class logger_factory
    // --------------------------------------------------------------------------------------------------
    logger_factory::logger_factory( std::string log_pattern,
                                    spdlog::level::level_enum log_level,
                                    std::vector<spdlog::sink_ptr> sinks )
        : log_pattern_{std::move( log_pattern )}, log_level_{log_level}, sinks_{std::move( sinks )}
    {
    }

    auto logger_factory::create( const cxxopts::ParseResult& config ) -> logger_factory
    {
        // TODO: set log queue size from command line param
        const auto log_q_size = 4096;
        spdlog::set_async_mode( log_q_size );

        const auto log_pattern = "[%Y-%m-%d %H:%M:%S.%f] [%l] [%P/%t] [%n] %v";
        const auto log_level =
            safe_log_level_filter( config[context::LOG_LEVEL].as<std::string>( ), spdlog::level::level_enum::info );
        auto sinks = std::vector<spdlog::sink_ptr>{};
        if ( config[context::LOG_DISABLE].as<bool>( ) )
        {
            sinks.push_back( std::make_shared<spdlog::sinks::null_sink_mt>( ) );
        }
        else
        {
            const auto log_file_name = config[context::LOG_FILE].as<std::string>( );
            sinks.push_back( std::make_shared<spdlog::sinks::simple_file_sink_mt>( log_file_name ) );

            if ( config[context::LOG_CONSOLE].as<bool>( ) )
            {
                sinks.push_back( std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>( ) );
            }
        }

        return logger_factory{log_pattern, log_level, sinks};
    }

    auto logger_factory::disabled( ) -> logger_factory
    {
        auto sinks = std::vector<spdlog::sink_ptr>{};
        sinks.push_back( std::make_shared<spdlog::sinks::null_sink_mt>( ) );

        return logger_factory{"", spdlog::level::off, sinks};
    }

    auto logger_factory::logger( const std::string& logger_name ) const -> std::unique_ptr<spdlog::logger>
    {
        auto logger = std::make_unique<spdlog::logger>( logger_name, std::begin( sinks_ ), std::end( sinks_ ) );
        logger->set_pattern( log_pattern_ );
        logger->set_level( log_level_ );
        logger->flush_on( spdlog::level::err );

        return logger;
    }
}  // namespace io_wally::logging
