//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_PROTOCOL_IMPL_COMMON_MESSAGES_IPP
#define BOOST_MYSQL_DETAIL_PROTOCOL_IMPL_COMMON_MESSAGES_IPP

#pragma once

#include <boost/mysql/client_errc.hpp>
#include <boost/mysql/common_server_errc.hpp>
#include <boost/mysql/mariadb_server_errc.hpp>
#include <boost/mysql/mysql_server_errc.hpp>

#include <boost/mysql/detail/auxiliar/server_errc_strings.hpp>
#include <boost/mysql/detail/protocol/common_messages.hpp>

inline boost::mysql::detail::deserialize_errc boost::mysql::detail::serialization_traits<
    boost::mysql::detail::ok_packet,
    boost::mysql::detail::serialization_tag::struct_with_fields>::
    deserialize_(deserialization_context& ctx, ok_packet& output) noexcept
{
    {
        auto err = deserialize(
            ctx,
            output.affected_rows,
            output.last_insert_id,
            output.status_flags,
            output.warnings
        );
        if (err == deserialize_errc::ok && ctx.enough_size(1))  // message is optional, may be omitted
        {
            err = deserialize(ctx, output.info);
        }
        return err;
    }
}

inline boost::mysql::detail::deserialize_errc boost::mysql::detail::serialization_traits<
    boost::mysql::detail::column_definition_packet,
    boost::mysql::detail::serialization_tag::struct_with_fields>::
    deserialize_(deserialization_context& ctx, column_definition_packet& output) noexcept
{
    int_lenenc length_of_fixed_fields;
    std::uint16_t final_padding = 0;
    return deserialize(
        ctx,
        output.catalog,
        output.schema,
        output.table,
        output.org_table,
        output.name,
        output.org_name,
        length_of_fixed_fields,
        output.character_set,
        output.column_length,
        output.type,
        output.flags,
        output.decimals,
        final_padding
    );
}

#endif /* INCLUDE_BOOST_MYSQL_DETAIL_PROTOCOL_IMPL_COMMON_MESSAGES_IPP_ */
