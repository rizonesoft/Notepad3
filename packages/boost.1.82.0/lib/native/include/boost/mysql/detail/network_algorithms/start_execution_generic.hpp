//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_START_EXECUTION_GENERIC_HPP
#define BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_START_EXECUTION_GENERIC_HPP

#include <boost/mysql/diagnostics.hpp>
#include <boost/mysql/error_code.hpp>
#include <boost/mysql/execution_state.hpp>

#include <boost/mysql/detail/auxiliar/field_type_traits.hpp>
#include <boost/mysql/detail/channel/channel.hpp>
#include <boost/mysql/detail/config.hpp>
#include <boost/mysql/detail/protocol/capabilities.hpp>
#include <boost/mysql/detail/protocol/common_messages.hpp>
#include <boost/mysql/detail/protocol/db_flavor.hpp>
#include <boost/mysql/detail/protocol/prepared_statement_messages.hpp>
#include <boost/mysql/detail/protocol/query_messages.hpp>
#include <boost/mysql/detail/protocol/resultset_encoding.hpp>

#include <cstddef>
#include <type_traits>
#include <utility>

#ifdef BOOST_MYSQL_HAS_CONCEPTS
#include <concepts>
#endif

namespace boost {
namespace mysql {
namespace detail {

// Exposed for the sake of testing
struct execute_response
{
    enum class type_t
    {
        num_fields,
        ok_packet,
        error
    } type;
    union data_t
    {
        static_assert(std::is_trivially_destructible<error_code>::value, "");

        std::size_t num_fields;
        ok_packet ok_pack;
        error_code err;

        data_t(size_t v) noexcept : num_fields(v) {}
        data_t(const ok_packet& v) noexcept : ok_pack(v) {}
        data_t(error_code v) noexcept : err(v) {}
    } data;

    execute_response(std::size_t v) noexcept : type(type_t::num_fields), data(v) {}
    execute_response(const ok_packet& v) noexcept : type(type_t::ok_packet), data(v) {}
    execute_response(error_code v) noexcept : type(type_t::error), data(v) {}
};
inline execute_response deserialize_execute_response(
    boost::asio::const_buffer msg,
    capabilities caps,
    db_flavor flavor,
    diagnostics& diag
) noexcept;

#ifdef BOOST_MYSQL_HAS_CONCEPTS

template <class T>
concept serialize_fn = std::invocable<std::decay_t<T>, capabilities, std::vector<std::uint8_t>&>;

#define BOOST_MYSQL_SERIALIZE_FN ::boost::mysql::detail::serialize_fn

#else  // BOOST_MYSQL_HAS_CONCEPTS

#define BOOST_MYSQL_SERIALIZE_FN class

#endif  // BOOST_MYSQL_HAS_CONCEPTS

// The sync version gets passed directlty the request packet to be serialized.
// There is no need to defer the serialization here.
template <class Stream, BOOST_MYSQL_SERIALIZE_FN SerializeFn>
void start_execution_generic(
    resultset_encoding encoding,
    channel<Stream>& channel,
    const SerializeFn& fn,
    execution_state& st,
    error_code& err,
    diagnostics& diag
);

// The async version gets passed a request maker, holding enough data to create
// the request packet when the async op is started. Used by statement execution
// with tuple params, to make lifetimes more user-friendly when using deferred tokens.
template <
    class Stream,
    BOOST_MYSQL_SERIALIZE_FN SerializeFn,
    BOOST_ASIO_COMPLETION_TOKEN_FOR(void(::boost::mysql::error_code)) CompletionToken>
BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken, void(error_code))
async_start_execution_generic(
    resultset_encoding encoding,
    channel<Stream>& chan,
    SerializeFn&& fn,
    execution_state& st,
    diagnostics& diag,
    CompletionToken&& token
);

}  // namespace detail
}  // namespace mysql
}  // namespace boost

#include <boost/mysql/detail/network_algorithms/impl/start_execution_generic.hpp>

#endif /* INCLUDE_MYSQL_IMPL_NETWORK_ALGORITHMS_READ_RESULTSET_HEAD_HPP_ */
