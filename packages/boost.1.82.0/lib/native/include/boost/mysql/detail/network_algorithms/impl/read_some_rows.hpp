//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_IMPL_READ_SOME_ROWS_HPP
#define BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_IMPL_READ_SOME_ROWS_HPP

#pragma once

#include <boost/mysql/diagnostics.hpp>
#include <boost/mysql/error_code.hpp>
#include <boost/mysql/execution_state.hpp>
#include <boost/mysql/row.hpp>

#include <boost/mysql/detail/network_algorithms/read_some_rows.hpp>
#include <boost/mysql/detail/protocol/deserialize_row.hpp>

#include <boost/asio/buffer.hpp>
#include <boost/asio/post.hpp>

#include <cstddef>

namespace boost {
namespace mysql {
namespace detail {

template <class Stream>
inline rows_view process_some_rows(
    channel<Stream>& channel,
    execution_state& st,
    error_code& err,
    diagnostics& diag
)
{
    // Process all read messages until they run out, an error happens
    // or an EOF is received
    std::size_t num_rows = 0;
    channel.shared_fields().clear();
    while (channel.has_read_messages())
    {
        // Get the row message
        auto message = channel.next_read_message(execution_state_access::get_sequence_number(st), err);
        if (err)
            return rows_view();

        // Deserialize the row. Values are stored in a vector owned by the channel
        deserialize_row(
            message,
            channel.current_capabilities(),
            channel.flavor(),
            channel.buffer_first(),
            st,
            channel.shared_fields(),
            err,
            diag
        );
        if (err)
            return rows_view();

        // There is no need to copy strings values anywhere; the returned values
        // will point into the channel's internal buffer

        // If we received an EOF, we're done
        if (st.complete())
            break;
        ++num_rows;
    }
    offsets_to_string_views(channel.shared_fields(), channel.buffer_first());

    return rows_view_access::construct(
        channel.shared_fields().data(),
        num_rows * st.meta().size(),
        st.meta().size()
    );
}

template <class Stream>
struct read_some_rows_op : boost::asio::coroutine
{
    channel<Stream>& chan_;
    diagnostics& diag_;
    execution_state& st_;

    read_some_rows_op(channel<Stream>& chan, diagnostics& diag, execution_state& st) noexcept
        : chan_(chan), diag_(diag), st_(st)
    {
    }

    template <class Self>
    void operator()(Self& self, error_code err = {})
    {
        // Error checking
        if (err)
        {
            self.complete(err, rows_view());
            return;
        }

        // Normal path
        rows_view output;
        BOOST_ASIO_CORO_REENTER(*this)
        {
            diag_.clear();

            // If the op is already complete, we don't need to read anything
            if (st_.complete())
            {
                BOOST_ASIO_CORO_YIELD boost::asio::post(std::move(self));
                self.complete(error_code(), rows_view());
                BOOST_ASIO_CORO_YIELD break;
            }

            // Read at least one message
            BOOST_ASIO_CORO_YIELD chan_.async_read_some(std::move(self));

            // Process messages
            output = process_some_rows(chan_, st_, err, diag_);

            self.complete(err, output);
        }
    }
};

}  // namespace detail
}  // namespace mysql
}  // namespace boost

template <class Stream>
boost::mysql::rows_view boost::mysql::detail::read_some_rows(
    channel<Stream>& channel,
    execution_state& st,
    error_code& err,
    diagnostics& diag
)
{
    // If the op is already complete, we don't need to read anything
    if (st.complete())
    {
        return rows_view();
    }

    // Read from the stream until there is at least one message
    channel.read_some(err);
    if (err)
        return rows_view();

    // Process read messages
    return process_some_rows(channel, st, err, diag);
}

template <
    class Stream,
    BOOST_ASIO_COMPLETION_TOKEN_FOR(void(::boost::mysql::error_code, ::boost::mysql::rows_view))
        CompletionToken>
BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken, void(boost::mysql::error_code, boost::mysql::rows_view))
boost::mysql::detail::async_read_some_rows(
    channel<Stream>& channel,
    execution_state& st,
    diagnostics& diag,
    CompletionToken&& token
)
{
    return boost::asio::async_compose<CompletionToken, void(error_code, rows_view)>(
        read_some_rows_op<Stream>(channel, diag, st),
        token,
        channel
    );
}

#endif /* INCLUDE_MYSQL_IMPL_NETWORK_ALGORITHMS_READ_TEXT_ROW_IPP_ */
