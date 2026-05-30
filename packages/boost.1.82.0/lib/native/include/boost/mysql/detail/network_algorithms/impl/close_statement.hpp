//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_IMPL_CLOSE_STATEMENT_HPP
#define BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_IMPL_CLOSE_STATEMENT_HPP

#pragma once

#include <boost/mysql/detail/network_algorithms/close_statement.hpp>
#include <boost/mysql/detail/protocol/prepared_statement_messages.hpp>

namespace boost {
namespace mysql {
namespace detail {

template <class Stream>
struct close_statement_op : boost::asio::coroutine
{
    channel<Stream>& chan_;
    std::uint32_t stmt_id_;
    diagnostics& diag_;

    close_statement_op(channel<Stream>& chan, const statement& stmt, diagnostics& diag) noexcept
        : chan_(chan), stmt_id_(stmt.id()), diag_(diag)
    {
    }

    template <class Self>
    void operator()(Self& self, error_code err = {})
    {
        // Error checking
        if (err)
        {
            self.complete(err);
            return;
        }

        // Regular coroutine body; if there has been an error, we don't get here
        BOOST_ASIO_CORO_REENTER(*this)
        {
            diag_.clear();

            // Serialize the close message
            serialize_message(
                com_stmt_close_packet{stmt_id_},
                chan_.current_capabilities(),
                chan_.shared_buffer()
            );

            // Write message (already serialized at this point)
            BOOST_ASIO_CORO_YIELD chan_
                .async_write(chan_.shared_buffer(), chan_.reset_sequence_number(), std::move(self));

            // Complete
            self.complete(error_code());
        }
    }
};

}  // namespace detail
}  // namespace mysql
}  // namespace boost

template <class Stream>
void boost::mysql::detail::
    close_statement(channel<Stream>& chan, const statement& stmt, error_code& code, diagnostics&)
{
    // Serialize the close message
    serialize_message(com_stmt_close_packet{stmt.id()}, chan.current_capabilities(), chan.shared_buffer());

    // Send it. No response is sent back
    chan.write(boost::asio::buffer(chan.shared_buffer()), chan.reset_sequence_number(), code);
}

template <class Stream, BOOST_ASIO_COMPLETION_TOKEN_FOR(void(::boost::mysql::error_code)) CompletionToken>
BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken, void(boost::mysql::error_code))
boost::mysql::detail::async_close_statement(
    channel<Stream>& chan,
    const statement& stmt,
    diagnostics& diag,
    CompletionToken&& token
)
{
    return boost::asio::async_compose<CompletionToken, void(error_code)>(
        close_statement_op<Stream>(chan, stmt, diag),
        token,
        chan
    );
}

#endif /* INCLUDE_BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_IMPL_CLOSE_STATEMENT_HPP_ */
