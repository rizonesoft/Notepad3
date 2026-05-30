//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_PROTOCOL_IMPL_PREPARED_STATEMENT_MESSAGES_HPP
#define BOOST_MYSQL_DETAIL_PROTOCOL_IMPL_PREPARED_STATEMENT_MESSAGES_HPP

#pragma once

#include <boost/mysql/detail/protocol/binary_serialization.hpp>
#include <boost/mysql/detail/protocol/null_bitmap_traits.hpp>
#include <boost/mysql/detail/protocol/prepared_statement_messages.hpp>

namespace boost {
namespace mysql {
namespace detail {

// Maps from an actual value to a protocol_field_type. Only value's type is used
inline protocol_field_type get_protocol_field_type(const field_view& input) noexcept
{
    switch (input.kind())
    {
    case field_kind::null: return protocol_field_type::null;
    case field_kind::int64: return protocol_field_type::longlong;
    case field_kind::uint64: return protocol_field_type::longlong;
    case field_kind::string: return protocol_field_type::string;
    case field_kind::blob: return protocol_field_type::blob;
    case field_kind::float_: return protocol_field_type::float_;
    case field_kind::double_: return protocol_field_type::double_;
    case field_kind::date: return protocol_field_type::date;
    case field_kind::datetime: return protocol_field_type::datetime;
    case field_kind::time: return protocol_field_type::time;
    default: assert(false); return protocol_field_type::null;
    }
}

// Whether to include the unsigned flag in the statement execute message
// for a given value or not. Only value's type is used
inline bool is_unsigned(const field_view& input) noexcept { return input.is_uint64(); }

}  // namespace detail
}  // namespace mysql
}  // namespace boost

inline boost::mysql::detail::deserialize_errc boost::mysql::detail::serialization_traits<
    boost::mysql::detail::com_stmt_prepare_ok_packet,
    boost::mysql::detail::serialization_tag::struct_with_fields>::
    deserialize_(deserialization_context& ctx, com_stmt_prepare_ok_packet& output) noexcept
{
    std::uint8_t reserved;
    return deserialize(
        ctx,
        output.statement_id,
        output.num_columns,
        output.num_params,
        reserved,
        output.warning_count
    );
}

template <class FieldViewFwdIterator>
inline std::size_t boost::mysql::detail::serialization_traits<
    boost::mysql::detail::com_stmt_execute_packet<FieldViewFwdIterator>,
    boost::mysql::detail::serialization_tag::struct_with_fields>::
    get_size_(
        const serialization_context& ctx,
        const com_stmt_execute_packet<FieldViewFwdIterator>& value
    ) noexcept
{
    std::size_t res = 1 +  // command ID
                      get_size(ctx, value.statement_id, value.flags, value.iteration_count);
    auto num_params = std::distance(value.params_begin, value.params_end);
    assert(num_params >= 0 && num_params <= 255);

    if (num_params > 0u)
    {
        res += null_bitmap_traits(stmt_execute_null_bitmap_offset, num_params).byte_count();
        res += get_size(ctx, value.new_params_bind_flag);
        res += get_size(ctx, com_stmt_execute_param_meta_packet{}) * num_params;
        for (auto it = value.params_begin; it != value.params_end; ++it)
        {
            res += get_size(ctx, *it);
        }
    }

    return res;
}

template <class FieldViewFwdIterator>
inline void boost::mysql::detail::serialization_traits<
    boost::mysql::detail::com_stmt_execute_packet<FieldViewFwdIterator>,
    boost::mysql::detail::serialization_tag::struct_with_fields>::
    serialize_(
        serialization_context& ctx,
        const com_stmt_execute_packet<FieldViewFwdIterator>& input
    ) noexcept
{
    constexpr std::uint8_t command_id = com_stmt_execute_packet<FieldViewFwdIterator>::command_id;
    serialize(ctx, command_id, input.statement_id, input.flags, input.iteration_count);

    // Number of parameters
    auto num_params = std::distance(input.params_begin, input.params_end);
    assert(num_params >= 0 && num_params <= 255);

    if (num_params > 0)
    {
        // NULL bitmap
        null_bitmap_traits traits(stmt_execute_null_bitmap_offset, num_params);
        std::size_t i = 0;
        std::memset(ctx.first(), 0, traits.byte_count());  // Initialize to zeroes
        for (auto it = input.params_begin; it != input.params_end; ++it, ++i)
        {
            if (it->is_null())
            {
                traits.set_null(ctx.first(), i);
            }
        }
        ctx.advance(traits.byte_count());

        // new parameters bind flag
        serialize(ctx, input.new_params_bind_flag);

        // value metadata
        com_stmt_execute_param_meta_packet meta;
        for (auto it = input.params_begin; it != input.params_end; ++it)
        {
            meta.type = get_protocol_field_type(*it);
            meta.unsigned_flag = is_unsigned(*it) ? 0x80 : 0;
            serialize(ctx, meta);
        }

        // actual values
        for (auto it = input.params_begin; it != input.params_end; ++it)
        {
            serialize(ctx, *it);
        }
    }
}

#endif
