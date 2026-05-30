//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_IMPL_CLOSE_CONNECTION_HPP
#define BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_IMPL_CLOSE_CONNECTION_HPP

#pragma once

#include <boost/mysql/detail/network_algorithms/close_connection.hpp>
#include <boost/mysql/detail/network_algorithms/quit_connection.hpp>

#include <boost/asio/post.hpp>

#include <type_traits>

namespace boost {
namespace mysql {
namespace detail {

template <class SocketStream>
struct close_connection_op : boost::asio::coroutine
{
    channel<SocketStream>& chan_;
    diagnostics& diag_;

    close_connection_op(channel<SocketStream>& chan, diagnostics& diag) : chan_(chan), diag_(diag) {}

    template <class Self>
    void operator()(Self& self, error_code err = {})
    {
        error_code close_err;
        BOOST_ASIO_CORO_REENTER(*this)
        {
            diag_.clear();

            if (!chan_.lowest_layer().is_open())
            {
                BOOST_ASIO_CORO_YIELD boost::asio::post(std::move(self));
                self.complete(error_code());
                BOOST_ASIO_CORO_YIELD break;
            }

            BOOST_ASIO_CORO_YIELD async_quit_connection(chan_, diag_, std::move(self));

            // We call close regardless of the quit outcome
            close_err = chan_.close();
            self.complete(err ? err : close_err);
        }
    }
};

}  // namespace detail
}  // namespace mysql
}  // namespace boost

template <class SocketStream>
void boost::mysql::detail::close_connection(channel<SocketStream>& chan, error_code& code, diagnostics& diag)
{
    // Close = quit + close stream. We close the stream regardless of the quit failing or not
    if (chan.lowest_layer().is_open())
    {
        // MySQL quit notification
        quit_connection(chan, code, diag);

        auto err = chan.close();
        if (!code)
        {
            code = err;
        }
    }
}

template <
    class SocketStream,
    BOOST_ASIO_COMPLETION_TOKEN_FOR(void(::boost::mysql::error_code)) CompletionToken>
BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken, void(boost::mysql::error_code))
boost::mysql::detail::async_close_connection(
    channel<SocketStream>& chan,
    diagnostics& diag,
    CompletionToken&& token
)
{
    return boost::asio::async_compose<CompletionToken, void(boost::mysql::error_code)>(
        close_connection_op<SocketStream>{chan, diag},
        token,
        chan
    );
}

#endif /* INCLUDE_BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_IMPL_CLOSE_CONNECTION_HPP_ */
