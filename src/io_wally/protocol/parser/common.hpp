#pragma once

#include <tuple>

#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include "io_wally/protocol/common.hpp"

using namespace io_wally::protocol;

namespace io_wally
{
    namespace protocol
    {
        namespace parser
        {
            enum parse_state : int
            {
                INCOMPLETE = 0,
                COMPLETE,
                MALFORMED_INPUT
            };

            class header_parser
            {
               public:
                template <typename InputIterator>
                struct result
                {
                   public:
                    result( const enum parse_state parse_state )
                        : parse_state_( parse_state ), parsed_header_( boost::none ), consumed_until_( boost::none )
                    {
                        return;
                    }

                    result( const uint8_t type_and_flags,
                            const uint32_t remaining_length,
                            const InputIterator consumed_until )
                        : parse_state_( COMPLETE ),
                          parsed_header_( packet::header( type_and_flags, remaining_length ) ),
                          consumed_until_( consumed_until )
                    {
                        return;
                    }

                    const enum parse_state parse_state( ) const
                    {
                        return parse_state_;
                    }

                    const bool is_parsing_complete( ) const
                    {
                        return ( parse_state_ == COMPLETE ) || ( parse_state_ == MALFORMED_INPUT );
                    }

                    const bool is_input_malformed( ) const
                    {
                        return parse_state_ == MALFORMED_INPUT;
                    }

                    const packet::header parsed_header( ) const
                    {
                        return parsed_header_.get( );
                    }

                    const InputIterator consumed_until( ) const
                    {
                        return consumed_until_.get( );
                    }

                   private:
                    const enum parse_state parse_state_;
                    const boost::optional<packet::header> parsed_header_;
                    const boost::optional<InputIterator> consumed_until_;
                };
                header_parser( ) : type_and_flags_( -1 ), remaining_length_( )
                {
                    return;
                }

                template <typename InputIterator>
                const result<InputIterator> parse( InputIterator buf_start, const InputIterator buf_end )
                {
                    uint32_t rem_len = -1;
                    packet::remaining_length::parse_state rem_len_pst = packet::remaining_length::INCOMPLETE;
                    while ( ( buf_start != buf_end ) && ( rem_len_pst == packet::remaining_length::INCOMPLETE ) )
                    {
                        if ( type_and_flags_ == -1 )
                        {
                            type_and_flags_ = *buf_start++;
                        }
                        else
                        {
                            rem_len_pst = remaining_length_( rem_len, *buf_start++ );
                        }
                    }

                    if ( rem_len_pst == packet::remaining_length::OUT_OF_RANGE )
                        return result<InputIterator>( MALFORMED_INPUT );
                    if ( rem_len_pst == packet::remaining_length::INCOMPLETE )
                        return result<InputIterator>( INCOMPLETE );

                    return result<InputIterator>( type_and_flags_, rem_len, buf_start );
                }

                void reset( )
                {
                    type_and_flags_ = -1;
                    remaining_length_.reset( );
                }

               private:
                /// Fields
                int16_t type_and_flags_;
                packet::remaining_length remaining_length_;
            };

            struct packet_parse_result
            {
               public:
                static const packet_parse_result* const ok( mqtt_packet* mqtt_packet )
                {
                    return new packet_parse_result( COMPLETE, mqtt_packet );
                }

                static const packet_parse_result* const malformed_input( )
                {
                    return new packet_parse_result( MALFORMED_INPUT, nullptr );
                }

                const enum parse_state parse_state( ) const
                {
                    return parse_state_;
                }

                std::unique_ptr<mqtt_packet> parsed_packet( )
                {
                    return std::move( parsed_packet_ );
                }

               private:
                packet_parse_result( const enum parse_state parse_state, mqtt_packet* mqtt_pkt )
                    : parse_state_( parse_state ), parsed_packet_( mqtt_pkt ){};

                ~packet_parse_result( )
                {
                    return;
                }

                /// Fields
                const enum parse_state parse_state_;
                std::unique_ptr<mqtt_packet> parsed_packet_;
            };

            template <typename InputIterator>
            class packet_body_parser
            {
               public:
                virtual ~packet_body_parser( )
                {
                    return;
                }

                virtual const std::unique_ptr<mqtt_packet> parse( const packet::header_flags& header_flags,
                                                                  InputIterator buf_start,
                                                                  const InputIterator buf_end ) = 0;
            };

            class mqtt_packet_parser
            {
               public:
                mqtt_packet_parser( ) : remaining_length_( )
                {
                    return;
                }

                template <typename InputIterator>
                const packet_parse_result* const parse( InputIterator buf_start, const InputIterator buf_end )
                {
                    /// TODO: Implement
                    return nullptr;
                }

                void reset( )
                {
                    remaining_length_.reset( );
                }

               private:
                /// Fields
                packet::remaining_length remaining_length_;
            };
        }  /// namespace parser
    }      /// namespace protocol
}  /// namespace io_wally
