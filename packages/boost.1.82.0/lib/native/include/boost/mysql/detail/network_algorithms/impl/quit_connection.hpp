//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_IMPL_QUIT_CONNECTION_HPP
#define BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_IMPL_QUIT_CONNECTION_HPP

#pragma once

#include <boost/mysql/diagnostics.hpp>
#include <boost/mysql/error_code.hpp>

#include <boost/mysql/detail/channel/channel.hpp>
#include <boost/mysql/detail/network_algorithms/quit_connection.hpp>

#include <boost/asio/coroutine.hpp>

namespace boost {
namespace mysql {
namespace detail {

template <class Stream>
void compose_quit(channel<Stream>& chan)
{
    serialize_message(quit_packet(), chan.current_capabilities(), chan.shared_buffer());
}

template <class Stream>
struct quit_connection_op : boost::asio::coroutine
{
    channel<Stream>& chan_;
    diagnostics& diag_;

    quit_connection_op(channel<Stream>& chan, diagnostics& diag) noexcept : chan_(chan), diag_(diag) {}

    template <class Self>
    void operator()(Self& self, error_code err = {})
    {
        BOOST_ASIO_CORO_REENTER(*this)
        {
            diag_.clear();

            // Quit message
            compose_quit(chan_);
            BOOST_ASIO_CORO_YIELD chan_
                .async_write(chan_.shared_buffer(), chan_.reset_sequence_number(), std::move(self));
            if (err)
            {
                self.complete(err);
            }

            // SSL shutdown error ignored, as MySQL doesn't always gracefully
            // close SSL connections. TODO: was this because of a missing if?
            if (chan_.stream().ssl_active())
            {
                BOOST_ASIO_CORO_YIELD chan_.stream().async_shutdown(std::move(self));
            }

            self.complete(error_code());
        }
    }
};

}  // namespace detail
}  // namespace mysql
}  // namespace boost

template <class Stream>
void boost::mysql::detail::quit_connection(channel<Stream>& chan, error_code& err, diagnostics&)
{
    compose_quit(chan);
    chan.write(chan.shared_buffer(), chan.reset_sequence_number(), err);
    if (err)
        return;
    if (chan.stream().ssl_active())
    {
        // SSL shutdown. Result ignored as MySQL does not always perform
        // graceful SSL shutdowns
        error_code ignored;
        chan.stream().shutdown(ignored);
    }
}

template <class Stream, BOOST_ASIO_COMPLETION_TOKEN_FOR(void(::boost::mysql::error_code)) CompletionToken>
BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken, void(boost::mysql::error_code))
boost::mysql::detail::async_quit_connection(channel<Stream>& chan, diagnostics& diag, CompletionToken&& token)
{
    return boost::asio::async_compose<CompletionToken, void(error_code)>(
        quit_connection_op<Stream>(chan, diag),
        token,
        chan
    );
}

#endif /* INCLUDE_BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_IMPL_QUIT_CONNECTION_HPP_ */
