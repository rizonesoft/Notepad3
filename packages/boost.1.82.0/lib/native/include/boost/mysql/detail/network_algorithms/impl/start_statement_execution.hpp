//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_IMPL_START_STATEMENT_EXECUTION_HPP
#define BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_IMPL_START_STATEMENT_EXECUTION_HPP

#pragma once

#include <boost/mysql/diagnostics.hpp>
#include <boost/mysql/error_code.hpp>
#include <boost/mysql/execution_state.hpp>
#include <boost/mysql/field_view.hpp>
#include <boost/mysql/statement.hpp>

#include <boost/mysql/detail/auxiliar/stringize.hpp>
#include <boost/mysql/detail/network_algorithms/start_execution_generic.hpp>
#include <boost/mysql/detail/network_algorithms/start_statement_execution.hpp>
#include <boost/mysql/detail/protocol/capabilities.hpp>
#include <boost/mysql/detail/protocol/prepared_statement_messages.hpp>
#include <boost/mysql/detail/protocol/resultset_encoding.hpp>
#include <boost/mysql/detail/protocol/serialization.hpp>

#include <boost/asio/compose.hpp>
#include <boost/asio/post.hpp>
#include <boost/mp11/integer_sequence.hpp>

#include <cstddef>
#include <cstdint>
#include <tuple>
#include <type_traits>

namespace boost {
namespace mysql {
namespace detail {

template <BOOST_MYSQL_FIELD_VIEW_FORWARD_ITERATOR FieldViewFwdIterator>
com_stmt_execute_packet<FieldViewFwdIterator> make_stmt_execute_packet(
    std::uint32_t stmt_id,
    FieldViewFwdIterator params_first,
    FieldViewFwdIterator params_last
) noexcept(std::is_nothrow_copy_constructible<FieldViewFwdIterator>::value)
{
    return com_stmt_execute_packet<FieldViewFwdIterator>{
        stmt_id,
        std::uint8_t(0),   // flags
        std::uint32_t(1),  // iteration count
        std::uint8_t(1),   // new params flag: set
        params_first,
        params_last,
    };
}

template <BOOST_MYSQL_FIELD_LIKE... T, std::size_t... I>
std::array<field_view, sizeof...(T)> tuple_to_array_impl(const std::tuple<T...>& t, boost::mp11::index_sequence<I...>) noexcept
{
    return std::array<field_view, sizeof...(T)>{{field_view(std::get<I>(t))...}};
}

template <BOOST_MYSQL_FIELD_LIKE... T>
std::array<field_view, sizeof...(T)> tuple_to_array(const std::tuple<T...>& t) noexcept
{
    return tuple_to_array_impl(t, boost::mp11::make_index_sequence<sizeof...(T)>());
}

template <BOOST_MYSQL_FIELD_VIEW_FORWARD_ITERATOR FieldViewFwdIterator>
class stmt_execute_it_serialize_fn
{
    std::uint32_t stmt_id_;
    FieldViewFwdIterator first_;
    FieldViewFwdIterator last_;

public:
    stmt_execute_it_serialize_fn(
        std::uint32_t stmt_id,
        FieldViewFwdIterator first,
        FieldViewFwdIterator last
    ) noexcept(std::is_nothrow_copy_constructible<FieldViewFwdIterator>::value)
        : stmt_id_(stmt_id), first_(first), last_(last)
    {
    }

    void operator()(capabilities caps, std::vector<std::uint8_t>& buffer) const
    {
        auto request = make_stmt_execute_packet(stmt_id_, first_, last_);
        serialize_message(request, caps, buffer);
    }
};

template <BOOST_MYSQL_FIELD_LIKE_TUPLE FieldLikeTuple>
class stmt_execute_tuple_serialize_fn
{
    std::uint32_t stmt_id_;
    FieldLikeTuple params_;

public:
    // We need a deduced context to enable perfect forwarding
    template <BOOST_MYSQL_FIELD_LIKE_TUPLE DeducedTuple>
    stmt_execute_tuple_serialize_fn(std::uint32_t stmt_id, DeducedTuple&& params) noexcept(
        std::is_nothrow_constructible<FieldLikeTuple, decltype(std::forward<DeducedTuple>(params))>::value
    )
        : stmt_id_(stmt_id), params_(std::forward<DeducedTuple>(params))
    {
    }

    void operator()(capabilities caps, std::vector<std::uint8_t>& buffer) const
    {
        auto field_views = tuple_to_array(params_);
        auto request = make_stmt_execute_packet(stmt_id_, field_views.begin(), field_views.end());
        serialize_message(request, caps, buffer);
    }
};

inline error_code check_num_params(const statement& stmt, std::size_t param_count)
{
    if (param_count != stmt.num_params())
    {
        return make_error_code(client_errc::wrong_num_params);
    }
    else
    {
        return error_code();
    }
}

template <BOOST_MYSQL_FIELD_VIEW_FORWARD_ITERATOR FieldViewFwdIterator>
error_code check_num_params(
    const statement& stmt,
    FieldViewFwdIterator params_first,
    FieldViewFwdIterator params_last
)
{
    return check_num_params(stmt, std::distance(params_first, params_last));
}

struct fast_fail_op : boost::asio::coroutine
{
    error_code err_;
    diagnostics& diag_;

    fast_fail_op(error_code err, diagnostics& diag) noexcept : err_(err), diag_(diag) {}

    template <class Self>
    void operator()(Self& self)
    {
        BOOST_ASIO_CORO_REENTER(*this)
        {
            BOOST_ASIO_CORO_YIELD boost::asio::post(std::move(self));
            diag_.clear();
            self.complete(err_);
        }
    }
};

}  // namespace detail
}  // namespace mysql
}  // namespace boost

template <class Stream, BOOST_MYSQL_FIELD_VIEW_FORWARD_ITERATOR FieldViewFwdIterator>
void boost::mysql::detail::start_statement_execution(
    channel<Stream>& chan,
    const statement& stmt,
    FieldViewFwdIterator params_first,
    FieldViewFwdIterator params_last,
    execution_state& output,
    error_code& err,
    diagnostics& diag
)
{
    err = check_num_params(stmt, params_first, params_last);
    if (!err)
    {
        start_execution_generic(
            resultset_encoding::binary,
            chan,
            stmt_execute_it_serialize_fn<FieldViewFwdIterator>(stmt.id(), params_first, params_last),
            output,
            err,
            diag
        );
    }
}

template <
    class Stream,
    BOOST_MYSQL_FIELD_VIEW_FORWARD_ITERATOR FieldViewFwdIterator,
    BOOST_ASIO_COMPLETION_TOKEN_FOR(void(::boost::mysql::error_code)) CompletionToken>
BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken, void(boost::mysql::error_code))
boost::mysql::detail::async_start_statement_execution(
    channel<Stream>& chan,
    const statement& stmt,
    FieldViewFwdIterator params_first,
    FieldViewFwdIterator params_last,
    execution_state& output,
    diagnostics& diag,
    CompletionToken&& token
)
{
    error_code err = check_num_params(stmt, params_first, params_last);
    if (err)
    {
        return boost::asio::async_compose<CompletionToken, void(error_code)>(
            fast_fail_op(err, diag),
            token,
            chan
        );
    }
    return async_start_execution_generic(
        resultset_encoding::binary,
        chan,
        stmt_execute_it_serialize_fn<FieldViewFwdIterator>(stmt.id(), params_first, params_last),
        output,
        diag,
        std::forward<CompletionToken>(token)
    );
}

template <class Stream, BOOST_MYSQL_FIELD_LIKE_TUPLE FieldLikeTuple>
void boost::mysql::detail::start_statement_execution(
    channel<Stream>& channel,
    const statement& stmt,
    const FieldLikeTuple& params,
    execution_state& output,
    error_code& err,
    diagnostics& diag
)
{
    auto params_array = tuple_to_array(params);
    start_statement_execution(channel, stmt, params_array.begin(), params_array.end(), output, err, diag);
}

template <
    class Stream,
    BOOST_MYSQL_FIELD_LIKE_TUPLE FieldLikeTuple,
    BOOST_ASIO_COMPLETION_TOKEN_FOR(void(::boost::mysql::error_code)) CompletionToken>
BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken, void(boost::mysql::error_code))
boost::mysql::detail::async_start_statement_execution(
    channel<Stream>& chan,
    const statement& stmt,
    FieldLikeTuple&& params,
    execution_state& output,
    diagnostics& diag,
    CompletionToken&& token
)
{
    using decayed_tuple = typename std::decay<FieldLikeTuple>::type;
    error_code err = check_num_params(stmt, std::tuple_size<decayed_tuple>::value);
    if (err)
    {
        return boost::asio::async_compose<CompletionToken, void(error_code)>(
            fast_fail_op(err, diag),
            token,
            chan
        );
    }
    return async_start_execution_generic(
        resultset_encoding::binary,
        chan,
        stmt_execute_tuple_serialize_fn<decayed_tuple>(stmt.id(), std::forward<FieldLikeTuple>(params)),
        output,
        diag,
        std::forward<CompletionToken>(token)
    );
}

#endif /* INCLUDE_BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_IMPL_EXECUTE_STATEMENT_HPP_ */
