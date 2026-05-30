//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_CHANNEL_DISABLEABLE_SSL_STREAM_HPP
#define BOOST_MYSQL_DETAIL_CHANNEL_DISABLEABLE_SSL_STREAM_HPP

#include <boost/mysql/error_code.hpp>

#include <boost/asio/async_result.hpp>

#include <cstddef>

namespace boost {
namespace mysql {
namespace detail {

// A wrapper for a Stream that makes it possible to disable SSL
// at runtime. Satisfies the Stream concept. Required to implement SSL negotiation

template <class Stream>
class disableable_ssl_stream
{
public:
    template <class... Args>
    disableable_ssl_stream(Args&&... args) noexcept : inner_stream_(std::forward<Args>(args)...)
    {
    }

    void reset() noexcept { set_ssl_active(false); }  // TODO: do we really need this?
    bool ssl_active() const noexcept { return ssl_active_; }
    void set_ssl_active(bool v) noexcept { ssl_active_ = v; }

    using executor_type = typename Stream::executor_type;
    using next_layer_type = Stream;
    using lowest_layer_type = typename Stream::lowest_layer_type;

    executor_type get_executor() noexcept { return inner_stream_.get_executor(); }
    next_layer_type& next_layer() noexcept { return inner_stream_; }
    const next_layer_type& next_layer() const noexcept { return inner_stream_; }
    lowest_layer_type& lowest_layer() noexcept { return inner_stream_.lowest_layer(); }

    void handshake(error_code& ec);

    template <BOOST_ASIO_COMPLETION_TOKEN_FOR(void(::boost::mysql::error_code)) CompletionToken>
    BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(error_code))
    async_handshake(CompletionToken&& token);

    void shutdown(error_code& ec);

    template <BOOST_ASIO_COMPLETION_TOKEN_FOR(void(::boost::mysql::error_code)) CompletionToken>
    BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(error_code))
    async_shutdown(CompletionToken&& token);

    template <class MutableBufferSequence>
    std::size_t read_some(const MutableBufferSequence&, error_code& ec);

    template <
        class MutableBufferSequence,
        BOOST_ASIO_COMPLETION_TOKEN_FOR(void(::boost::mysql::error_code, std::size_t))
            CompletionToken>
    BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(error_code, std::size_t))
    async_read_some(const MutableBufferSequence& buff, CompletionToken&& token);

    template <class ConstBufferSequence>
    std::size_t write_some(const ConstBufferSequence&, error_code& ec);

    template <
        class ConstBufferSequence,
        BOOST_ASIO_COMPLETION_TOKEN_FOR(void(::boost::mysql::error_code, std::size_t))
            CompletionToken>
    BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(error_code, std::size_t))
    async_write_some(const ConstBufferSequence& buff, CompletionToken&& token);

private:
    Stream inner_stream_;
    bool ssl_active_{false};
};

}  // namespace detail
}  // namespace mysql
}  // namespace boost

#include <boost/mysql/detail/channel/impl/disableable_ssl_stream.hpp>

#endif /* INCLUDE_BOOST_MYSQL_DETAIL_AUXILIAR_STATIC_STRING_HPP_ */
