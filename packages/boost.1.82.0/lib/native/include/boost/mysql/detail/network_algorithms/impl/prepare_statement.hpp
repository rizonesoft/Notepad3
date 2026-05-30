//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_IMPL_PREPARE_STATEMENT_HPP
#define BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_IMPL_PREPARE_STATEMENT_HPP

#pragma once

#include <boost/mysql/diagnostics.hpp>
#include <boost/mysql/error_code.hpp>
#include <boost/mysql/statement.hpp>

#include <boost/mysql/detail/auxiliar/bytestring.hpp>
#include <boost/mysql/detail/channel/channel.hpp>
#include <boost/mysql/detail/network_algorithms/prepare_statement.hpp>
#include <boost/mysql/detail/protocol/capabilities.hpp>
#include <boost/mysql/detail/protocol/process_error_packet.hpp>
#include <boost/mysql/detail/protocol/serialization.hpp>

#include <boost/asio/buffer.hpp>

#include <cstdint>

namespace boost {
namespace mysql {
namespace detail {

class prepare_statement_processor
{
    channel_base& channel_;
    string_view stmt_sql_;
    diagnostics& diag_;
    statement res_;
    unsigned remaining_meta_{};

public:
    template <class Stream>
    prepare_statement_processor(channel<Stream>& chan, string_view stmt_sql, diagnostics& diag) noexcept
        : channel_(chan), stmt_sql_(stmt_sql), diag_(diag)
    {
    }

    void clear_diag() noexcept { diag_.clear(); }

    void process_request()
    {
        com_stmt_prepare_packet packet{string_eof(stmt_sql_)};
        serialize_message(packet, channel_.current_capabilities(), channel_.shared_buffer());
    }

    void process_response(boost::asio::const_buffer message, error_code& err)
    {
        deserialization_context ctx(message, channel_.current_capabilities());
        std::uint8_t msg_type = 0;
        err = deserialize_message_part(ctx, msg_type);
        if (err)
            return;

        if (msg_type == error_packet_header)
        {
            err = process_error_packet(ctx, channel_.flavor(), diag_);
        }
        else if (msg_type != 0)
        {
            err = make_error_code(client_errc::protocol_value_error);
        }
        else
        {
            com_stmt_prepare_ok_packet response{};
            err = deserialize_message(ctx, response);
            if (err)
                return;

            statement_access::reset(res_, response);
            remaining_meta_ = response.num_columns + response.num_params;
        }
    }

    bool has_remaining_meta() const noexcept { return remaining_meta_ != 0; }
    void on_meta_received() noexcept { --remaining_meta_; }
    const statement& result() const noexcept { return res_; }
    channel_base& get_channel() noexcept { return channel_; }
};

template <class Stream>
struct prepare_statement_op : boost::asio::coroutine
{
    prepare_statement_processor processor_;

    prepare_statement_op(channel<Stream>& chan, string_view stmt_sql, diagnostics& diag)
        : processor_(chan, stmt_sql, diag)
    {
    }

    channel<Stream>& get_channel() noexcept
    {
        return static_cast<channel<Stream>&>(processor_.get_channel());
    }

    template <class Self>
    void operator()(Self& self, error_code err = {}, boost::asio::const_buffer read_message = {})
    {
        // Error checking
        if (err)
        {
            self.complete(err, statement());
            return;
        }

        // Regular coroutine body; if there has been an error, we don't get here
        BOOST_ASIO_CORO_REENTER(*this)
        {
            processor_.clear_diag();

            // Serialize request
            processor_.process_request();

            // Write message
            BOOST_ASIO_CORO_YIELD get_channel().async_write(
                get_channel().shared_buffer(),
                get_channel().reset_sequence_number(),
                std::move(self)
            );

            // Read response
            BOOST_ASIO_CORO_YIELD get_channel().async_read_one(
                get_channel().shared_sequence_number(),
                std::move(self)
            );

            // Process response
            processor_.process_response(read_message, err);
            if (err)
            {
                self.complete(err, statement());
                BOOST_ASIO_CORO_YIELD break;
            }

            // Server sends now one packet per parameter and field.
            // We ignore these for now.
            while (processor_.has_remaining_meta())
            {
                // Read from the stream if necessary
                if (!get_channel().has_read_messages())
                {
                    BOOST_ASIO_CORO_YIELD get_channel().async_read_some(std::move(self));
                }

                // Read the message
                read_message = get_channel().next_read_message(get_channel().shared_sequence_number(), err);
                if (err)
                {
                    self.complete(err, statement());
                    BOOST_ASIO_CORO_YIELD break;
                }

                // Note it as processed
                processor_.on_meta_received();
            }

            // Complete
            self.complete(error_code(), processor_.result());
        }
    }
};

}  // namespace detail
}  // namespace mysql
}  // namespace boost

template <class Stream>
boost::mysql::statement boost::mysql::detail::prepare_statement(
    channel<Stream>& channel,
    string_view stmt_sql,
    error_code& err,
    diagnostics& diag
)
{
    prepare_statement_processor processor(channel, stmt_sql, diag);

    // Prepare message
    processor.process_request();

    // Write message
    channel.write(channel.shared_buffer(), channel.reset_sequence_number(), err);
    if (err)
        return statement();

    // Read response
    auto read_buffer = channel.read_one(channel.shared_sequence_number(), err);
    if (err)
        return statement();

    // Process response
    processor.process_response(read_buffer, err);
    if (err)
        return statement();

    // Server sends now one packet per parameter and field.
    // We ignore these for now.
    while (processor.has_remaining_meta())
    {
        // Read from the stream if necessary
        if (!channel.has_read_messages())
        {
            channel.read_some(err);
            if (err)
                return statement();
        }

        // Discard the message
        channel.next_read_message(channel.shared_sequence_number(), err);
        if (err)
            return statement();

        // Update the processor state
        processor.on_meta_received();
    }

    return processor.result();
}

template <
    class Stream,
    BOOST_ASIO_COMPLETION_TOKEN_FOR(void(::boost::mysql::error_code, ::boost::mysql::statement))
        CompletionToken>
BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken, void(boost::mysql::error_code, boost::mysql::statement))
boost::mysql::detail::async_prepare_statement(
    channel<Stream>& chan,
    string_view stmt_sql,
    diagnostics& diag,
    CompletionToken&& token
)
{
    return boost::asio::async_compose<CompletionToken, void(error_code, statement)>(
        prepare_statement_op<Stream>(chan, stmt_sql, diag),
        token,
        chan
    );
}

#endif /* INCLUDE_BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_IMPL_PREPARE_STATEMENT_HPP_ */
