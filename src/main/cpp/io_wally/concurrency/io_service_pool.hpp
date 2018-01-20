#pragma once

#include <cassert>
#include <memory>
#include <thread>
#include <vector>
#include <atomic>
#include <functional>
#include <mutex>
#include <condition_variable>

#include <asio.hpp>

#include <boost/log/common.hpp>
#include <boost/log/trivial.hpp>

namespace io_wally
{
    /// \brief Namespace facilitating concurrency support.
    ///
    /// Contains
    ///
    ///  - \c io_service_pool  A pool of \c ::asio::io_service instances, each executing in a dedicated thread
    namespace concurrency
    {
        /// \brief Pool of \c ::asio::io_service objects, each executing in a dedicated thread.
        ///
        /// \see http://www.boost.org/doc/libs/1_53_0/doc/html/boost_asio/example/http/server2/io_service_pool.cpp
        class io_service_pool final
        {
           public:
            io_service_pool( const std::string& name, std::size_t pool_size = 1 )
                : name_{name},
                  pool_size_{pool_size},
                  logger_{boost::log::keywords::channel = "io-srvc-pool:" + name,
                          boost::log::keywords::severity = boost::log::trivial::trace}
            {
                assert( pool_size > 0 );
                for ( std::size_t i = 0; i < pool_size; ++i )
                {
                    auto io_service = std::make_shared<::asio::io_service>( );
                    auto work = std::make_shared<::asio::io_service::work>( *io_service );
                    io_services_.push_back( io_service );
                    work_.push_back( work );
                }
            }

            /// \brief This \c io_service_pool's name.
            ///
            /// \return This \c io_service_pool's name
            const std::string& name( ) const
            {
                return name_;
            }

            /// \brief Run all \c io_service objects, each in its dedicated thread, and return immediately.
            void run( )
            {
                BOOST_LOG_SEV( logger_, boost::log::trivial::info ) << "START:   IO service pool [" << name_
                                                                    << "|threads:" << pool_size_ << "]";
                for ( std::size_t i = 0; i < pool_size_; ++i )
                {
                    // see:
                    // http://stackoverflow.com/questions/28794203/why-does-boostasioio-service-not-compile-with-stdbind
                    auto th = std::make_shared<std::thread>( std::bind(
                        static_cast<std::size_t (::asio::io_service::*)( )>( &::asio::io_service::run ),
                        io_services_[i] ) );
                    threads_.push_back( th );
                }
                BOOST_LOG_SEV( logger_, boost::log::trivial::info ) << "STARTED: IO service pool [" << name_
                                                                    << "|threads:" << pool_size_ << "]";
            }

            /// \brief Stop all \c io_service objects, and return immediately.
            ///
            /// This method blocks until all threads have terminated.
            void stop( )
            {
                BOOST_LOG_SEV( logger_, boost::log::trivial::info ) << "STOPPING: IO service pool [" << name_
                                                                    << "|threads:" << pool_size_ << "]";
                for ( auto& work : work_ )
                    work.reset( );
                for ( auto& ios : io_services_ )
                    ios->stop( );
                for ( auto& th : threads_ )
                {
                    if ( th->joinable( ) )
                        th->join( );
                }
                {
                    auto ul = std::unique_lock<std::mutex>{stop_mutex_};
                    running_ = false;
                    stopped_.notify_all( );
                }
                BOOST_LOG_SEV( logger_, boost::log::trivial::info ) << "STOPPED: IO service pool [" << name_
                                                                    << "|threads:" << pool_size_ << "]";
            }

            /// \brief Get next available \c io_service object in pool.
            ///
            /// \return Next \c io_service object in this pool
            ::asio::io_service& io_service( )
            {
                auto next = next_io_service_++;
                // TODO: This is just an ill-fated attempt to show off! Get rid of it!
                if ( next >= pool_size_ )
                {
                    // This does not GUARANTUEE that next_io_service_ will not grow boundless. It should, however, be
                    // highly unlikely.
                    auto seen = std::size_t{0};
                    do
                    {
                        auto seen = next_io_service_.load( );
                        if ( seen < pool_size_ )  // Some other helpful thread came along
                            break;
                    } while ( next_io_service_.compare_exchange_strong( seen, seen % pool_size_ ) );
                }

                return *io_services_[next % pool_size_];
            }

            void wait_until_stopped( )
            {
                auto ul = std::unique_lock<std::mutex>{stop_mutex_};
                stopped_.wait( ul,
                               [this]( )
                               {
                    return !running_;
                } );
            }

           private:  // static
            using io_service_ptr = std::shared_ptr<::asio::io_service>;
            using work_ptr = std::shared_ptr<::asio::io_service::work>;
            using thread_ptr = std::shared_ptr<std::thread>;

           private:
            const std::string name_;
            const std::size_t pool_size_;
            std::vector<io_service_ptr> io_services_{};
            std::vector<work_ptr> work_{};
            std::vector<thread_ptr> threads_{};
            std::atomic_size_t next_io_service_{0};
            bool running_{true};
            /// Signal when we have been stopped and all threads have terminated
            std::mutex stop_mutex_{};
            std::condition_variable stopped_{};
            /// Our severity-enabled channel logger
            boost::log::sources::severity_channel_logger<boost::log::trivial::severity_level> logger_;
        };  // class io_service_pool
    }       // namespace concurrency
}  // namespace io_wally
