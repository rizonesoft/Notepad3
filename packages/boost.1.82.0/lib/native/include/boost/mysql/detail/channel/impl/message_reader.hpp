//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_CHANNEL_IMPL_MESSAGE_READER_HPP
#define BOOST_MYSQL_DETAIL_CHANNEL_IMPL_MESSAGE_READER_HPP

#pragma once

#include <boost/mysql/detail/auxiliar/valgrind.hpp>
#include <boost/mysql/detail/channel/message_reader.hpp>

#include <boost/asio/buffer.hpp>
#include <boost/asio/compose.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/post.hpp>

boost::asio::const_buffer boost::mysql::detail::message_reader::get_next_message(
    std::uint8_t& seqnum,
    error_code& ec
) noexcept
{
    assert(has_message());
    if (result_.message.has_seqnum_mismatch || seqnum != result_.message.seqnum_first)
    {
        ec = make_error_code(client_errc::sequence_number_mismatch);
        return {};
    }
    seqnum = result_.message.seqnum_last + 1;
    boost::asio::const_buffer res(
        buffer_.current_message_first() - result_.message.size,
        result_.message.size
    );
    parse_message();
    return res;
}

template <class Stream>
void boost::mysql::detail::message_reader::read_some(
    Stream& stream,
    error_code& ec,
    bool keep_messages
)
{
    // If we already have a message, complete immediately
    if (has_message())
        return;

    // Remove processed messages if we can
    if (!keep_messages)
        buffer_.remove_reserved();

    while (!has_message())
    {
        // If any previous process_message indicated that we need more
        // buffer space, resize the buffer now
        maybe_resize_buffer();

        // Actually read bytes
        std::size_t bytes_read = stream.read_some(buffer_.free_area(), ec);
        if (ec)
            break;
        valgrind_make_mem_defined(buffer_.free_first(), bytes_read);

        // Process them
        on_read_bytes(bytes_read);
    }
}

template <class Stream>
struct boost::mysql::detail::message_reader::read_some_op : boost::asio::coroutine
{
    message_reader& reader_;
    bool keep_messages_;
    Stream& stream_;

    read_some_op(message_reader& reader, bool keep_messages, Stream& stream) noexcept
        : reader_(reader), keep_messages_(keep_messages), stream_(stream)
    {
    }

    template <class Self>
    void operator()(Self& self, error_code ec = {}, std::size_t bytes_read = 0)
    {
        // Error handling
        if (ec)
        {
            self.complete(ec);
            return;
        }

        // Non-error path
        BOOST_ASIO_CORO_REENTER(*this)
        {
            // If we already have a message, complete immediately
            if (reader_.has_message())
            {
                BOOST_ASIO_CORO_YIELD boost::asio::post(std::move(self));
                self.complete(error_code());
                BOOST_ASIO_CORO_YIELD break;
            }

            // Remove processed messages if we can
            if (!keep_messages_)
                reader_.buffer_.remove_reserved();

            while (!reader_.has_message())
            {
                // If any previous process_message indicated that we need more
                // buffer space, resize the buffer now
                reader_.maybe_resize_buffer();

                // Actually read bytes
                BOOST_ASIO_CORO_YIELD stream_.async_read_some(
                    reader_.buffer_.free_area(),
                    std::move(self)
                );
                valgrind_make_mem_defined(reader_.buffer_.free_first(), bytes_read);

                // Process them
                reader_.on_read_bytes(bytes_read);
            }

            self.complete(error_code());
        }
    }
};

template <
    class Stream,
    BOOST_ASIO_COMPLETION_TOKEN_FOR(void(::boost::mysql::error_code)) CompletionToken>
BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken, void(::boost::mysql::error_code))
boost::mysql::detail::message_reader::async_read_some(
    Stream& stream,
    CompletionToken&& token,
    bool keep_messages
)
{
    return boost::asio::async_compose<CompletionToken, void(error_code)>(
        read_some_op<Stream>{*this, keep_messages, stream},
        token,
        stream
    );
}

template <class Stream>
boost::asio::const_buffer boost::mysql::detail::message_reader::read_one(
    Stream& stream,
    std::uint8_t& seqnum,
    error_code& ec,
    bool keep_messages
)
{
    read_some(stream, ec, keep_messages);
    if (ec)
        return {};
    else
        return get_next_message(seqnum, ec);
}

template <class Stream>
struct boost::mysql::detail::message_reader::read_one_op : boost::asio::coroutine
{
    message_reader& reader_;
    bool keep_messages_;
    Stream& stream_;
    std::uint8_t& seqnum_;

    read_one_op(message_reader& reader, bool keep_messages, Stream& stream, std::uint8_t& seqnum)
        : reader_(reader), keep_messages_(keep_messages), stream_(stream), seqnum_(seqnum)
    {
    }

    template <class Self>
    void operator()(Self& self, error_code code = {})
    {
        // Error handling
        if (code)
        {
            self.complete(code, boost::asio::const_buffer());
            return;
        }

        // Non-error path
        boost::asio::const_buffer b;
        BOOST_ASIO_CORO_REENTER(*this)
        {
            BOOST_ASIO_CORO_YIELD reader_.async_read_some(stream_, std::move(self), keep_messages_);
            b = reader_.get_next_message(seqnum_, code);
            self.complete(code, b);
        }
    }
};

template <
    class Stream,
    BOOST_ASIO_COMPLETION_TOKEN_FOR(void(::boost::mysql::error_code, boost::asio::const_buffer))
        CompletionToken>
BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(
    CompletionToken,
    void(boost::mysql::error_code, ::boost::asio::const_buffer)
)
boost::mysql::detail::message_reader::async_read_one(
    Stream& stream,
    std::uint8_t& seqnum,
    CompletionToken&& token,
    bool keep_messages
)
{
    return boost::asio::async_compose<CompletionToken, void(error_code, boost::asio::const_buffer)>(
        read_one_op<Stream>{*this, keep_messages, stream, seqnum},
        token,
        stream
    );
}

void boost::mysql::detail::message_reader::maybe_resize_buffer()
{
    if (!result_.has_message)
    {
        buffer_.grow_to_fit(result_.required_size);
    }
}

void boost::mysql::detail::message_reader::on_read_bytes(size_t num_bytes)
{
    buffer_.move_to_pending(num_bytes);
    parse_message();
}

#endif
