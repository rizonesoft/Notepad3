//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_PROTOCOL_IMPL_SERIALIZATION_IPP
#define BOOST_MYSQL_DETAIL_PROTOCOL_IMPL_SERIALIZATION_IPP

#pragma once

#include <boost/mysql/detail/protocol/serialization.hpp>

#include <limits>

namespace boost {
namespace mysql {
namespace detail {

inline string_view get_string(const std::uint8_t* from, std::size_t size)
{
    return string_view(reinterpret_cast<const char*>(from), size);
}

}  // namespace detail
}  // namespace mysql
}  // namespace boost

inline boost::mysql::detail::deserialize_errc boost::mysql::detail::serialization_traits<
    boost::mysql::detail::int_lenenc,
    boost::mysql::detail::serialization_tag::none>::
    deserialize_(deserialization_context& ctx, int_lenenc& output) noexcept
{
    std::uint8_t first_byte = 0;
    auto err = deserialize(ctx, first_byte);
    if (err != deserialize_errc::ok)
    {
        return err;
    }

    if (first_byte == 0xFC)
    {
        std::uint16_t value = 0;
        err = deserialize(ctx, value);
        output.value = value;
    }
    else if (first_byte == 0xFD)
    {
        int3 value;
        err = deserialize(ctx, value);
        output.value = value.value;
    }
    else if (first_byte == 0xFE)
    {
        std::uint64_t value = 0;
        err = deserialize(ctx, value);
        output.value = value;
    }
    else
    {
        err = deserialize_errc::ok;
        output.value = first_byte;
    }
    return err;
}

inline void boost::mysql::detail::serialization_traits<
    boost::mysql::detail::int_lenenc,
    boost::mysql::detail::serialization_tag::none>::
    serialize_(serialization_context& ctx, int_lenenc input) noexcept
{
    if (input.value < 251)
    {
        serialize(ctx, static_cast<std::uint8_t>(input.value));
    }
    else if (input.value < 0x10000)
    {
        ctx.write(0xfc);
        serialize(ctx, static_cast<std::uint16_t>(input.value));
    }
    else if (input.value < 0x1000000)
    {
        ctx.write(0xfd);
        serialize(ctx, int3(static_cast<std::uint32_t>(input.value)));
    }
    else
    {
        ctx.write(0xfe);
        serialize(ctx, static_cast<std::uint64_t>(input.value));
    }
}

inline std::size_t boost::mysql::detail::serialization_traits<
    boost::mysql::detail::int_lenenc,
    boost::mysql::detail::serialization_tag::none>::
    get_size_(const serialization_context&, int_lenenc input) noexcept
{
    if (input.value < 251)
        return 1;
    else if (input.value < 0x10000)
        return 3;
    else if (input.value < 0x1000000)
        return 4;
    else
        return 9;
}

inline boost::mysql::detail::deserialize_errc boost::mysql::detail::serialization_traits<
    boost::mysql::detail::string_null,
    boost::mysql::detail::serialization_tag::none>::
    deserialize_(deserialization_context& ctx, string_null& output) noexcept
{
    auto string_end = std::find(ctx.first(), ctx.last(), 0);
    if (string_end == ctx.last())
    {
        return deserialize_errc::incomplete_message;
    }
    output.value = get_string(ctx.first(), string_end - ctx.first());
    ctx.set_first(string_end + 1);  // skip the null terminator
    return deserialize_errc::ok;
}

inline boost::mysql::detail::deserialize_errc boost::mysql::detail::serialization_traits<
    boost::mysql::detail::string_eof,
    boost::mysql::detail::serialization_tag::none>::
    deserialize_(deserialization_context& ctx, string_eof& output) noexcept
{
    output.value = get_string(ctx.first(), ctx.last() - ctx.first());
    ctx.set_first(ctx.last());
    return deserialize_errc::ok;
}

inline boost::mysql::detail::deserialize_errc boost::mysql::detail::serialization_traits<
    boost::mysql::detail::string_lenenc,
    boost::mysql::detail::serialization_tag::none>::
    deserialize_(deserialization_context& ctx, string_lenenc& output) noexcept
{
    int_lenenc length;
    auto err = deserialize(ctx, length);
    if (err != deserialize_errc::ok)
    {
        return err;
    }
    if (length.value > std::numeric_limits<std::size_t>::max())
    {
        return deserialize_errc::protocol_value_error;
    }
    auto len = static_cast<std::size_t>(length.value);
    if (!ctx.enough_size(len))
    {
        return deserialize_errc::incomplete_message;
    }

    output.value = get_string(ctx.first(), len);
    ctx.advance(len);
    return deserialize_errc::ok;
}

#endif /* INCLUDE_BOOST_MYSQL_DETAIL_PROTOCOL_IMPL_SERIALIZATION_IPP_ */
