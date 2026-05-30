//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_IMPL_STATEMENT_HPP
#define BOOST_MYSQL_IMPL_STATEMENT_HPP

#pragma once

#include <boost/mysql/statement.hpp>

#include <boost/mysql/detail/auxiliar/access_fwd.hpp>
#include <boost/mysql/detail/protocol/prepared_statement_messages.hpp>

struct boost::mysql::detail::statement_access
{
    static void reset(statement& stmt, const detail::com_stmt_prepare_ok_packet& msg) noexcept
    {
        stmt.valid_ = true;
        stmt.id_ = msg.statement_id;
        stmt.num_params_ = msg.num_params;
    }
};

#endif
