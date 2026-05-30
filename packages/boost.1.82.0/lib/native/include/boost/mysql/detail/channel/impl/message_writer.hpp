//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_CHANNEL_IMPL_MESSAGE_WRITER_HPP
#define BOOST_MYSQL_DETAIL_CHANNEL_IMPL_MESSAGE_WRITER_HPP

#pragma once

#include <boost/mysql/detail/channel/message_writer.hpp>

#include <boost/asio/compose.hpp>
#include <boost/asio/coroutine.hpp>

template <class Stream>
void boost::mysql::detail::message_writer::write(
    Stream& stream,
    boost::asio::const_buffer buffer,
    std::uint8_t& seqnum,
    error_code& ec
)
{
    processor_.reset(buffer, seqnum);

    do
    {
        boost::asio::write(stream, processor_.prepare_next_chunk(), ec);
        if (ec)
            break;
        processor_.on_bytes_written();
    } while (!processor_.is_complete());
}

template <class Stream>
struct boost::mysql::detail::message_writer::write_op : boost::asio::coroutine
{
    Stream& stream_;
    message_writer_processor& processor_;
    boost::asio::const_buffer buffer_;
    std::uint8_t& seqnum_;

    write_op(
        Stream& stream,
        message_writer_processor& processor,
        boost::asio::const_buffer buffer,
        std::uint8_t& seqnum
    ) noexcept
        : stream_(stream), processor_(processor), buffer_(buffer), seqnum_(seqnum)
    {
    }

    template <class Self>
    void operator()(Self& self, error_code ec = {}, std::size_t = 0)
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
            processor_.reset(buffer_, seqnum_);

            // is_complete never returns false after a call to reset(), so no post() needed

            do
            {
                BOOST_ASIO_CORO_YIELD boost::asio::async_write(
                    stream_,
                    processor_.prepare_next_chunk(),
                    std::move(self)
                );
                processor_.on_bytes_written();
            } while (!processor_.is_complete());

            self.complete(error_code());
        }
    }
};

template <
    class Stream,
    BOOST_ASIO_COMPLETION_TOKEN_FOR(void(::boost::mysql::error_code)) CompletionToken>
BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken, void(boost::mysql::error_code))
boost::mysql::detail::message_writer::async_write(
    Stream& stream,
    boost::asio::const_buffer buffer,
    std::uint8_t& seqnum,
    CompletionToken&& token
)
{
    return boost::asio::async_compose<CompletionToken, void(error_code)>(
        write_op<Stream>(stream, processor_, buffer, seqnum),
        token,
        stream
    );
}

#endif
