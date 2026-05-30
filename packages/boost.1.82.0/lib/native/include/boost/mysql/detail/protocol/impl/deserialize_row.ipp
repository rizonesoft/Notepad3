//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_PROTOCOL_IMPL_DESERIALIZE_ROW_IPP
#define BOOST_MYSQL_DETAIL_PROTOCOL_IMPL_DESERIALIZE_ROW_IPP

#pragma once

#include <boost/mysql/detail/protocol/deserialize_binary_field.hpp>
#include <boost/mysql/detail/protocol/deserialize_row.hpp>
#include <boost/mysql/detail/protocol/deserialize_text_field.hpp>
#include <boost/mysql/detail/protocol/null_bitmap_traits.hpp>
#include <boost/mysql/detail/protocol/process_error_packet.hpp>
#include <boost/mysql/detail/protocol/serialization.hpp>

namespace boost {
namespace mysql {
namespace detail {

inline bool is_next_field_null(const deserialization_context& ctx)
{
    if (!ctx.enough_size(1))
        return false;
    return *ctx.first() == 0xfb;
}

inline error_code deserialize_text_row(
    deserialization_context& ctx,
    const std::vector<metadata>& fields,
    const std::uint8_t* buffer_first,
    std::vector<field_view>& output
)
{
    // Make space
    std::size_t old_size = output.size();
    auto num_fields = fields.size();
    output.resize(old_size + num_fields);

    for (std::vector<field_view>::size_type i = 0; i < num_fields; ++i)
    {
        if (is_next_field_null(ctx))
        {
            ctx.advance(1);
            output[old_size + i] = field_view(nullptr);
        }
        else
        {
            string_lenenc value_str;
            auto err = deserialize(ctx, value_str);
            if (err != deserialize_errc::ok)
                return to_error_code(err);
            err = deserialize_text_field(value_str.value, fields[i], buffer_first, output[old_size + i]);
            if (err != deserialize_errc::ok)
                return to_error_code(err);
        }
    }
    if (!ctx.empty())
        return client_errc::extra_bytes;
    return error_code();
}

inline error_code deserialize_binary_row(
    deserialization_context& ctx,
    const std::vector<metadata>& meta,
    const std::uint8_t* buffer_first,
    std::vector<field_view>& output
)
{
    // Skip packet header (it is not part of the message in the binary
    // protocol but it is in the text protocol, so we include it for homogeneity)
    // The caller will have checked we have this byte already for us
    assert(ctx.enough_size(1));
    ctx.advance(1);

    // Number of fields
    std::size_t old_size = output.size();
    auto num_fields = meta.size();
    output.resize(old_size + num_fields);

    // Null bitmap
    null_bitmap_traits null_bitmap(binary_row_null_bitmap_offset, num_fields);
    const std::uint8_t* null_bitmap_begin = ctx.first();
    if (!ctx.enough_size(null_bitmap.byte_count()))
        return client_errc::incomplete_message;
    ctx.advance(null_bitmap.byte_count());

    // Actual values
    for (std::vector<field_view>::size_type i = 0; i < num_fields; ++i)
    {
        if (null_bitmap.is_null(null_bitmap_begin, i))
        {
            output[old_size + i] = field_view(nullptr);
        }
        else
        {
            auto err = deserialize_binary_field(ctx, meta[i], buffer_first, output[old_size + i]);
            if (err != deserialize_errc::ok)
                return to_error_code(err);
        }
    }

    // Check for remaining bytes
    if (!ctx.empty())
        return make_error_code(client_errc::extra_bytes);

    return error_code();
}

}  // namespace detail
}  // namespace mysql
}  // namespace boost

void boost::mysql::detail::deserialize_row(
    resultset_encoding encoding,
    deserialization_context& ctx,
    const std::vector<metadata>& meta,
    const std::uint8_t* buffer_first,
    std::vector<field_view>& output,
    error_code& err
)
{
    err = encoding == detail::resultset_encoding::text
              ? deserialize_text_row(ctx, meta, buffer_first, output)
              : deserialize_binary_row(ctx, meta, buffer_first, output);
}

void boost::mysql::detail::deserialize_row(
    boost::asio::const_buffer read_message,
    capabilities current_capabilities,
    db_flavor flavor,
    const std::uint8_t* buffer_first,  // to store strings as offsets and allow buffer reallocation
    execution_state& st,
    std::vector<field_view>& output,
    error_code& err,
    diagnostics& diag
)
{
    assert(!st.complete());

    // Message type: row, error or eof?
    std::uint8_t msg_type = 0;
    deserialization_context ctx(read_message, current_capabilities);
    auto deser_errc = deserialize(ctx, msg_type);
    if (deser_errc != deserialize_errc::ok)
    {
        err = to_error_code(deser_errc);
        return;
    }

    if (msg_type == eof_packet_header)
    {
        // end of resultset => this is a ok_packet, not a row
        ok_packet ok_pack;
        err = deserialize_message(ctx, ok_pack);
        if (err)
            return;
        execution_state_access::complete(st, ok_pack);
    }
    else if (msg_type == error_packet_header)
    {
        // An error occurred during the generation of the rows
        err = process_error_packet(ctx, flavor, diag);
    }
    else
    {
        // An actual row
        ctx.rewind(1);  // keep the 'message type' byte, as it is part of the actual message
        deserialize_row(
            execution_state_access::get_encoding(st),
            ctx,
            execution_state_access::get_metadata(st),
            buffer_first,
            output,
            err
        );
    }
}

#endif
