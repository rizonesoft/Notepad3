//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_IMPL_START_QUERY_HPP
#define BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_IMPL_START_QUERY_HPP

#pragma once

#include <boost/mysql/string_view.hpp>

#include <boost/mysql/detail/network_algorithms/start_execution_generic.hpp>
#include <boost/mysql/detail/network_algorithms/start_query.hpp>
#include <boost/mysql/detail/protocol/capabilities.hpp>
#include <boost/mysql/detail/protocol/protocol_types.hpp>
#include <boost/mysql/detail/protocol/query_messages.hpp>
#include <boost/mysql/detail/protocol/resultset_encoding.hpp>
#include <boost/mysql/detail/protocol/serialization.hpp>

namespace boost {
namespace mysql {
namespace detail {

class query_serialize_fn
{
    string_view query_;

public:
    query_serialize_fn(string_view query) noexcept : query_(query) {}
    void operator()(capabilities caps, std::vector<std::uint8_t>& buffer) const
    {
        com_query_packet request{string_eof(query_)};
        serialize_message(request, caps, buffer);
    }
};

}  // namespace detail
}  // namespace mysql
}  // namespace boost

template <class Stream>
void boost::mysql::detail::start_query(
    channel<Stream>& channel,
    string_view query,
    execution_state& output,
    error_code& err,
    diagnostics& diag
)
{
    start_execution_generic(resultset_encoding::text, channel, query_serialize_fn(query), output, err, diag);
}

template <class Stream, BOOST_ASIO_COMPLETION_TOKEN_FOR(void(::boost::mysql::error_code)) CompletionToken>
BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken, void(boost::mysql::error_code))
boost::mysql::detail::async_start_query(
    channel<Stream>& chan,
    string_view query,
    execution_state& output,
    diagnostics& diag,
    CompletionToken&& token
)
{
    return async_start_execution_generic(
        resultset_encoding::text,
        chan,
        query_serialize_fn(query),
        output,
        diag,
        std::forward<CompletionToken>(token)
    );
}

#endif /* INCLUDE_BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_IMPL_EXECUTE_QUERY_HPP_ */
