//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_IMPL_EXECUTE_STATEMENT_HPP
#define BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_IMPL_EXECUTE_STATEMENT_HPP

#pragma once

#include <boost/mysql/results.hpp>
#include <boost/mysql/statement.hpp>

#include <boost/mysql/detail/auxiliar/field_type_traits.hpp>
#include <boost/mysql/detail/network_algorithms/execute_statement.hpp>
#include <boost/mysql/detail/network_algorithms/read_all_rows.hpp>
#include <boost/mysql/detail/network_algorithms/start_statement_execution.hpp>

#include <type_traits>

namespace boost {
namespace mysql {
namespace detail {

template <class Stream, BOOST_MYSQL_FIELD_LIKE_TUPLE FieldLikeTuple>
struct execute_statement_op : boost::asio::coroutine
{
    channel<Stream>& chan_;
    diagnostics& diag_;
    statement stmt_;
    FieldLikeTuple params_;
    results& output_;

    // We need a deduced context to enable perfect forwarding
    template <BOOST_MYSQL_FIELD_LIKE_TUPLE DeducedTuple>
    execute_statement_op(
        channel<Stream>& chan,
        diagnostics& diag,
        const statement& stmt,
        DeducedTuple&& params,
        results& output
    ) noexcept
        : chan_(chan), diag_(diag), stmt_(stmt), params_(std::forward<DeducedTuple>(params)), output_(output)
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
            BOOST_ASIO_CORO_YIELD
            async_start_statement_execution(
                chan_,
                stmt_,
                std::move(params_),
                results_access::get_state(output_),
                diag_,
                std::move(self)
            );

            BOOST_ASIO_CORO_YIELD async_read_all_rows(
                chan_,
                results_access::get_state(output_),
                results_access::get_rows(output_),
                diag_,
                std::move(self)
            );

            self.complete(error_code());
        }
    }
};

}  // namespace detail
}  // namespace mysql
}  // namespace boost

template <class Stream, BOOST_MYSQL_FIELD_LIKE_TUPLE FieldLikeTuple>
void boost::mysql::detail::execute_statement(
    channel<Stream>& channel,
    const statement& stmt,
    const FieldLikeTuple& params,
    results& output,
    error_code& err,
    diagnostics& diag
)
{
    start_statement_execution(channel, stmt, params, results_access::get_state(output), err, diag);
    if (err)
        return;

    read_all_rows(channel, results_access::get_state(output), results_access::get_rows(output), err, diag);
}

template <
    class Stream,
    BOOST_MYSQL_FIELD_LIKE_TUPLE FieldLikeTuple,
    BOOST_ASIO_COMPLETION_TOKEN_FOR(void(::boost::mysql::error_code)) CompletionToken>
BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken, void(boost::mysql::error_code))
boost::mysql::detail::async_execute_statement(
    channel<Stream>& chan,
    const statement& stmt,
    FieldLikeTuple&& params,
    results& output,
    diagnostics& diag,
    CompletionToken&& token
)
{
    using decayed_tuple = typename std::decay<FieldLikeTuple>::type;
    return boost::asio::async_compose<CompletionToken, void(boost::mysql::error_code)>(
        execute_statement_op<Stream, decayed_tuple>(
            chan,
            diag,
            stmt,
            std::forward<FieldLikeTuple>(params),
            output
        ),
        token,
        chan
    );
}

#endif /* INCLUDE_BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_IMPL_EXECUTE_QUERY_HPP_ */
