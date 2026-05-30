//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_CHANNEL_IMPL_DISABLEABLE_SSL_STREAM_HPP
#define BOOST_MYSQL_DETAIL_CHANNEL_IMPL_DISABLEABLE_SSL_STREAM_HPP

#pragma once

#include <boost/mysql/error_code.hpp>

#include <boost/mysql/detail/channel/disableable_ssl_stream.hpp>

#include <boost/asio/async_result.hpp>
#include <boost/asio/compose.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/ssl/stream_base.hpp>

#include <cstddef>
#include <utility>

namespace boost {
namespace mysql {
namespace detail {

template <class Stream>
using is_ssl_stream = std::is_base_of<boost::asio::ssl::stream_base, Stream>;

// Helpers to get the first non-SSL stream. We can't call just next_layer()
// because raw TCP sockets don't support this function. For non-SSL connections,
// we just return the stream itself
template <bool is_ssl_stream>
struct get_non_ssl_stream_t;

template <>
struct get_non_ssl_stream_t<true>
{
    template <typename Stream>
    static auto call(Stream& s) -> decltype(s.next_layer())
    {
        return s.next_layer();
    }
};

template <>
struct get_non_ssl_stream_t<false>
{
    template <typename Stream>
    static Stream& call(Stream& s) noexcept
    {
        return s;
    }
};

template <typename Stream>
auto get_non_ssl_stream(Stream& s)
    -> decltype(get_non_ssl_stream_t<is_ssl_stream<Stream>::value>::call(s))
{
    return get_non_ssl_stream_t<is_ssl_stream<Stream>::value>::call(s);
}

// A no-op operation that can be passed to async_compose
struct async_compose_noop
{
    template <class... Args>
    async_compose_noop(Args&&...) noexcept
    {
    }

    template <class... Args>
    void operator()(Args&&...)
    {
        assert(false);
    }
};

// Helpers to implement handshake
template <class Stream, bool supports_ssl>
struct ssl_handshake_helper;

template <class Stream>
struct ssl_handshake_helper<Stream, true>
{
    static void call(disableable_ssl_stream<Stream>& stream, error_code& ec)
    {
        stream.next_layer().handshake(boost::asio::ssl::stream_base::client, ec);
        if (!ec)
            stream.set_ssl_active(true);
    }
};

template <class Stream>
struct ssl_handshake_helper<Stream, false>
{
    static void call(disableable_ssl_stream<Stream>&, error_code&)
    {
        assert(false);  // should never be called
    }
};

template <class Stream, bool supports_ssl>
struct ssl_handshake_op;

template <class Stream>
struct ssl_handshake_op<Stream, true> : boost::asio::coroutine
{
    disableable_ssl_stream<Stream>& stream_;

    ssl_handshake_op(disableable_ssl_stream<Stream>& stream) noexcept : stream_(stream) {}

    template <class Self>
    void operator()(Self& self, error_code err = {})
    {
        // Error checking
        if (err)
        {
            self.complete(err);
            return;
        }

        // Non-error path
        BOOST_ASIO_CORO_REENTER(*this)
        {
            BOOST_ASIO_CORO_YIELD stream_.next_layer().async_handshake(
                boost::asio::ssl::stream_base::client,
                std::move(self)
            );
            stream_.set_ssl_active(true);
            self.complete(error_code());
        }
    }
};

template <class Stream>
struct ssl_handshake_op<Stream, false> : async_compose_noop
{
    using async_compose_noop::async_compose_noop;
};

// Helpers to implement shutdown
template <class Stream, bool supports_ssl>
struct ssl_shutdown_helper;

template <class Stream>
struct ssl_shutdown_helper<Stream, true>
{
    static void call_sync(disableable_ssl_stream<Stream>& stream, error_code& ec)
    {
        stream.next_layer().shutdown(ec);
    }

    template <BOOST_ASIO_COMPLETION_TOKEN_FOR(void(error_code)) CompletionToken>
    static BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(error_code))
        call_async(disableable_ssl_stream<Stream>& stream, CompletionToken&& token)
    {
        return stream.next_layer().async_shutdown(std::forward<CompletionToken>(token));
    }
};

template <class Stream>
struct ssl_shutdown_helper<Stream, false>
{
    static void call_sync(disableable_ssl_stream<Stream>&, error_code&)
    {
        assert(false);  // should never be called
    }

    template <BOOST_ASIO_COMPLETION_TOKEN_FOR(void(error_code)) CompletionToken>
    static BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(error_code))
        call_async(disableable_ssl_stream<Stream>& stream, CompletionToken&& token)
    {
        return boost::asio::async_compose<CompletionToken, void(error_code)>(
            async_compose_noop(),
            token,
            stream
        );
    }
};

}  // namespace detail
}  // namespace mysql
}  // namespace boost

template <class Stream>
void boost::mysql::detail::disableable_ssl_stream<Stream>::handshake(error_code& ec)
{
    ssl_handshake_helper<Stream, is_ssl_stream<Stream>::value>::call(*this, ec);
}

template <class Stream>
template <BOOST_ASIO_COMPLETION_TOKEN_FOR(void(::boost::mysql::error_code)) CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(boost::mysql::error_code))
boost::mysql::detail::disableable_ssl_stream<Stream>::async_handshake(CompletionToken&& token)
{
    return boost::asio::async_compose<CompletionToken, void(error_code)>(
        ssl_handshake_op<Stream, is_ssl_stream<Stream>::value>(*this),
        token,
        *this
    );
}

template <class Stream>
void boost::mysql::detail::disableable_ssl_stream<Stream>::shutdown(error_code& ec)
{
    ssl_shutdown_helper<Stream, is_ssl_stream<Stream>::value>::call_sync(*this, ec);
}

template <class Stream>
template <BOOST_ASIO_COMPLETION_TOKEN_FOR(void(::boost::mysql::error_code)) CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(boost::mysql::error_code))
boost::mysql::detail::disableable_ssl_stream<Stream>::async_shutdown(CompletionToken&& token)
{
    return ssl_shutdown_helper<Stream, is_ssl_stream<Stream>::value>::call_async(
        *this,
        std::forward<CompletionToken>(token)
    );
}

template <class Stream>
template <class MutableBufferSequence>
std::size_t boost::mysql::detail::disableable_ssl_stream<Stream>::read_some(
    const MutableBufferSequence& buff,
    error_code& ec
)
{
    if (ssl_active_)
    {
        return inner_stream_.read_some(buff, ec);
    }
    else
    {
        return get_non_ssl_stream(inner_stream_).read_some(buff, ec);
    }
}

template <class Stream>
template <
    class MutableBufferSequence,
    BOOST_ASIO_COMPLETION_TOKEN_FOR(void(::boost::mysql::error_code, std::size_t)) CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(boost::mysql::error_code, std::size_t))
boost::mysql::detail::disableable_ssl_stream<Stream>::async_read_some(
    const MutableBufferSequence& buff,
    CompletionToken&& token
)
{
    if (ssl_active_)
    {
        return inner_stream_.async_read_some(buff, std::forward<CompletionToken>(token));
    }
    else
    {
        return get_non_ssl_stream(inner_stream_)
            .async_read_some(buff, std::forward<CompletionToken>(token));
    }
}

template <class Stream>
template <class ConstBufferSequence>
std::size_t boost::mysql::detail::disableable_ssl_stream<Stream>::write_some(
    const ConstBufferSequence& buff,
    error_code& ec
)
{
    if (ssl_active_)
    {
        return inner_stream_.write_some(buff, ec);
    }
    else
    {
        return get_non_ssl_stream(inner_stream_).write_some(buff, ec);
    }
}

template <class Stream>
template <
    class ConstBufferSequence,
    BOOST_ASIO_COMPLETION_TOKEN_FOR(void(::boost::mysql::error_code, std::size_t)) CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(boost::mysql::error_code, std::size_t))
boost::mysql::detail::disableable_ssl_stream<Stream>::async_write_some(
    const ConstBufferSequence& buff,
    CompletionToken&& token
)
{
    if (ssl_active_)
    {
        return inner_stream_.async_write_some(buff, std::forward<CompletionToken>(token));
    }
    else
    {
        return get_non_ssl_stream(inner_stream_)
            .async_write_some(buff, std::forward<CompletionToken>(token));
    }
}

#endif
