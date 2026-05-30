//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_IMPL_READ_ALL_ROWS_HPP
#define BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_IMPL_READ_ALL_ROWS_HPP

#pragma once

#include <boost/mysql/diagnostics.hpp>
#include <boost/mysql/error_code.hpp>
#include <boost/mysql/execution_state.hpp>
#include <boost/mysql/rows_view.hpp>

#include <boost/mysql/detail/network_algorithms/read_all_rows.hpp>
#include <boost/mysql/detail/protocol/deserialize_row.hpp>

#include <boost/asio/buffer.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/post.hpp>

#include <cstddef>

namespace boost {
namespace mysql {
namespace detail {

template <class Stream>
inline void process_all_rows(
    channel<Stream>& channel,
    execution_state& st,
    rows& output,
    error_code& err,
    diagnostics& diag
)
{
    // Process all read messages until they run out, an error happens
    // or an EOF is received
    while (channel.has_read_messages())
    {
        // Get the row message
        auto message = channel.next_read_message(execution_state_access::get_sequence_number(st), err);
        if (err)
            return;

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
            return;

        // If we received an EOF, we're done
        if (st.complete())
        {
            offsets_to_string_views(channel.shared_fields(), channel.buffer_first());
            output = rows_view_access::construct(
                channel.shared_fields().data(),
                channel.shared_fields().size(),
                st.meta().size()
            );
            break;
        }
    }
}

template <class Stream>
struct read_all_rows_op : boost::asio::coroutine
{
    channel<Stream>& chan_;
    diagnostics& diag_;
    execution_state& st_;
    rows& output_;

    read_all_rows_op(channel<Stream>& chan, diagnostics& diag, execution_state& st, rows& output) noexcept
        : chan_(chan), diag_(diag), st_(st), output_(output)
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

        // Normal path
        BOOST_ASIO_CORO_REENTER(*this)
        {
            diag_.clear();
            rows_access::clear(output_);

            // If the op is already complete, we don't need to read anything
            if (st_.complete())
            {
                BOOST_ASIO_CORO_YIELD boost::asio::post(std::move(self));
                self.complete(error_code());
                BOOST_ASIO_CORO_YIELD break;
            }

            // Clear anything from previous runs
            chan_.shared_fields().clear();

            // Read at least one message
            while (!st_.complete())
            {
                // Actually read
                BOOST_ASIO_CORO_YIELD chan_.async_read_some(std::move(self), true);

                // Process messages
                process_all_rows(chan_, st_, output_, err, diag_);
                if (err)
                {
                    self.complete(err);
                    BOOST_ASIO_CORO_YIELD break;
                }
            }

            // Done
            self.complete(error_code());
        }
    }
};

}  // namespace detail
}  // namespace mysql
}  // namespace boost

template <class Stream>
void boost::mysql::detail::read_all_rows(
    channel<Stream>& channel,
    execution_state& st,
    rows& output,
    error_code& err,
    diagnostics& diag
)
{
    diag.clear();
    rows_access::clear(output);

    // If the op is already complete, we don't need to read anything
    if (st.complete())
    {
        err = error_code();
        return;
    }

    // Clear anything from previous runs
    channel.shared_fields().clear();

    while (!st.complete())
    {
        // Read from the stream until there is at least one message
        channel.read_some(err, true);
        if (err)
            return;

        // Process read messages
        process_all_rows(channel, st, output, err, diag);
        if (err)
            return;
    }
}

template <class Stream, BOOST_ASIO_COMPLETION_TOKEN_FOR(void(::boost::mysql::error_code)) CompletionToken>
BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken, void(boost::mysql::error_code))
boost::mysql::detail::async_read_all_rows(
    channel<Stream>& channel,
    execution_state& st,
    rows& output,
    diagnostics& diag,
    CompletionToken&& token
)
{
    return boost::asio::async_compose<CompletionToken, void(error_code)>(
        read_all_rows_op<Stream>(channel, diag, st, output),
        token,
        channel
    );
}

#endif /* INCLUDE_MYSQL_IMPL_NETWORK_ALGORITHMS_READ_TEXT_ROW_IPP_ */
