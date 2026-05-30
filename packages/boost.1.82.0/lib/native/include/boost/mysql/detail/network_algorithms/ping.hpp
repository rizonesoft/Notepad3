//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_PING_HPP
#define BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_PING_HPP

#include <boost/mysql/diagnostics.hpp>
#include <boost/mysql/error_code.hpp>

#include <boost/mysql/detail/channel/channel.hpp>
#include <boost/mysql/detail/protocol/capabilities.hpp>
#include <boost/mysql/detail/protocol/db_flavor.hpp>

#include <boost/asio/async_result.hpp>
#include <boost/asio/buffer.hpp>

namespace boost {
namespace mysql {
namespace detail {

// Exposed for the sake of testing
inline error_code process_ping_response(
    boost::asio::const_buffer msg,
    capabilities caps,
    db_flavor flavor,
    diagnostics& diag
);

template <class Stream>
void ping(channel<Stream>& channel, error_code& err, diagnostics& diag);

template <class Stream, BOOST_ASIO_COMPLETION_TOKEN_FOR(void(::boost::mysql::error_code)) CompletionToken>
BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken, void(error_code))
async_ping(channel<Stream>& chan, diagnostics& diag, CompletionToken&& token);

}  // namespace detail
}  // namespace mysql
}  // namespace boost

#include <boost/mysql/detail/network_algorithms/impl/ping.hpp>

#endif
