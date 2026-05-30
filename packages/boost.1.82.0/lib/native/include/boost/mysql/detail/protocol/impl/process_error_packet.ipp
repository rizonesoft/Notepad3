//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_PROTOCOL_IMPL_PROCESS_ERROR_PACKET_IPP
#define BOOST_MYSQL_DETAIL_PROTOCOL_IMPL_PROCESS_ERROR_PACKET_IPP

#pragma once

#include <boost/mysql/detail/auxiliar/server_errc_strings.hpp>
#include <boost/mysql/detail/protocol/common_messages.hpp>
#include <boost/mysql/detail/protocol/process_error_packet.hpp>

inline boost::mysql::error_code boost::mysql::detail::process_error_packet(
    deserialization_context& ctx,
    db_flavor flavor,
    diagnostics& diag
)
{
    err_packet error_packet{};
    auto code = deserialize_message(ctx, error_packet);
    if (code)
        return code;

    // Error message
    string_view sv = error_packet.error_message.value;
    diagnostics_access::assign(diag, sv);

    // Error code
    if (get_common_error_message(error_packet.error_code))
    {
        // This is an error shared between MySQL and MariaDB, represented as a common_server_errc.
        // get_common_error_message will check that the code has a common_server_errc representation
        // (the common error range has "holes" because of removed error codes)
        return make_error_code(static_cast<common_server_errc>(error_packet.error_code));
    }
    else
    {
        // This is a MySQL or MariaDB specific code. There is no fixed list of error codes,
        // as they both keep adding more codes, so no validation happens.
        const auto& cat = flavor == db_flavor::mysql ? get_mysql_server_category()
                                                     : get_mariadb_server_category();
        return error_code(error_packet.error_code, cat);
    }
}

#endif
