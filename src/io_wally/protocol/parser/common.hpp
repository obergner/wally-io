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
                    }
                    result( const uint8_t type_and_flags,
                            const uint32_t remaining_length,
                            const InputIterator consumed_until )
                        : parse_state_( COMPLETE ),
                          parsed_header_( packet::header( type_and_flags, remaining_length ) ),
                          consumed_until_( consumed_until )
                    {
                    }

                    const enum parse_state parse_state( ) const
                    {
                        return parse_state_;
                    }

                    const bool is_parsing_complete( ) const
                    {
                        return parse_state_ == COMPLETE;
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
                int16_t type_and_flags_;
                packet::remaining_length remaining_length_;
            };

            class packet_parser
            {
               public:
                packet_parser( ) : remaining_length_( )
                {
                }

                /*
                template <typename InputIterator>
                boost::variant<parse_state, std::tuple<mqtt_packet, InputIterator>> parse( InputIterator buf_start,
                                                                                           const InputIterator buf_end )
                {
                    return std::make_tuple( INCOMPLETE, new mqtt_packet( 0x80, 32400 ) );
                }
                */

               private:
                packet::remaining_length remaining_length_;
            };
        }  /// namespace parser
    }      /// namespace protocol
}  /// namespace io_wally
