#include <iostream>
#include <fstream>

#include <io_wally/application.hpp>
#include <io_wally/logging.hpp>
#include <io_wally/context.hpp>
#include <io_wally/impl/accept_all_authentication_service_factory.cpp>

namespace io_wally
{

    int application::run( int argc, char** argv )
    {
        try
        {
            const pair<const options::variables_map, const options::options_description> config_plus_desc =
                parse_configuration( argc, argv );
            const options::variables_map config = config_plus_desc.first;
            const options::options_description all_opts_desc = config_plus_desc.second;

            if ( config.count( "help" ) )
            {
                cout << "MQTT 3.1.1 server" << endl << all_opts_desc << endl;
                return application::EC_OK;
            }

            configure_logging( config );

            unique_ptr<io_wally::spi::authentication_service_factory> auth_service_factory =
                get_authentication_service_factory( config );

            const io_wally::context ctx( move( config ), move( auth_service_factory ) );
        }
        catch ( const options::error& e )
        {
            cerr << "Wrong usage: " << e.what( ) << endl;
            return application::EC_MALFORMED_CMDLINE;
        }
        catch ( const std::exception& e )
        {
            cerr << "Error: " << e.what( ) << endl;
            return application::EC_RUNTIME_ERROR;
        }
        return application::EC_OK;
    }

    const pair<const options::variables_map, const options::options_description> application::parse_configuration(
        int argc,
        char** argv ) const
    {
        options::variables_map config;

        options::options_description cmd_line_opts( "Command line" );
        cmd_line_opts.add_options( )( "help", "Print help message and exit" )(
            "config,c",
            options::value<string>( )->default_value( "/etc/mqttd.conf" ),
            "Use configuration from <file>" );

        options::options_description logging_opts( "Logging" );
        logging_opts.add_options( )(
            "logfile", options::value<string>( )->default_value( "/var/log/mqttd.log" ), "Log to file <file>" );

        options::options_description connection_opts( "Connection" );
        connection_opts.add_options( )(
            "connect-timeout",
            options::value<uint32_t>( )->default_value( application::DEFAULT_CONNECT_TIMEOUT_MS ),
            "Connect timeout in ms" )(
            "read-buffer-size",
            options::value<size_t>( )->default_value( application::DEFAULT_INITIAL_READ_BUFFER_SIZE ),
            "Initial read buffer size in bytes" )(
            "write-buffer-size",
            options::value<size_t>( )->default_value( application::DEFAULT_INITIAL_WRITE_BUFFER_SIZE ),
            "Initial write buffer size in bytes" );

        options::options_description all( "Allowed options" );
        all.add( cmd_line_opts ).add( logging_opts ).add( connection_opts );
        options::store( options::parse_command_line( argc, argv, all ), config );

        options::options_description config_file( "Config file options" );
        config_file.add( logging_opts ).add( connection_opts );
        ifstream config_fstream( config["logfile"].as<string>( ).c_str( ) );
        options::store( options::parse_config_file( config_fstream, config_file ), config );
        config_fstream.close( );  // TODO: Should be made exception-safe

        return make_pair( config, all );
    }

    void application::configure_logging( const options::variables_map& config ) const
    {
        io_wally::logging::init_logging( config["logfile"].as<const string>( ), true );

        return;
    }

    unique_ptr<io_wally::spi::authentication_service_factory> application::get_authentication_service_factory(
        const options::variables_map& /*  config */ ) const
    {
        return unique_ptr<io_wally::spi::authentication_service_factory>(
            new io_wally::impl::accept_all_authentication_service_factory( ) );
    }
}
