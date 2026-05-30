//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_PROTOCOL_IMPL_HANDSHAKE_MESSAGES_IPP
#define BOOST_MYSQL_DETAIL_PROTOCOL_IMPL_HANDSHAKE_MESSAGES_IPP

#pragma once

#include <boost/mysql/detail/protocol/handshake_messages.hpp>

inline boost::mysql::detail::deserialize_errc boost::mysql::detail::serialization_traits<
    boost::mysql::detail::handshake_packet,
    boost::mysql::detail::serialization_tag::struct_with_fields>::
    deserialize_(deserialization_context& ctx, handshake_packet& output) noexcept
{
    constexpr std::uint8_t auth1_length = 8;
    string_fixed<auth1_length> auth_plugin_data_part_1;
    string_fixed<2> capability_flags_low;
    string_fixed<2> capability_flags_high;
    std::uint8_t filler;  // should be 0
    std::uint8_t auth_plugin_data_len = 0;
    string_fixed<10> reserved;

    auto err = deserialize(
        ctx,
        output.server_version,
        output.connection_id,
        auth_plugin_data_part_1,
        filler,
        capability_flags_low,
        output.character_set,
        output.status_flags,
        capability_flags_high
    );
    if (err != deserialize_errc::ok)
        return err;

    // Compose capabilities
    auto capabilities_begin = reinterpret_cast<std::uint8_t*>(&output.capability_falgs);
    memcpy(capabilities_begin, capability_flags_low.data(), 2);
    memcpy(capabilities_begin + 2, capability_flags_high.data(), 2);
    boost::endian::little_to_native_inplace(output.capability_falgs);

    // Check minimum server capabilities to deserialize this frame
    capabilities cap(output.capability_falgs);
    if (!cap.has(CLIENT_PLUGIN_AUTH))
        return deserialize_errc::server_unsupported;

    // Deserialize following fields
    err = deserialize(ctx, auth_plugin_data_len, reserved);
    if (err != deserialize_errc::ok)
        return err;

    // Auth plugin data, second part
    auto auth2_length = static_cast<std::uint8_t>((std::max
    )(13, auth_plugin_data_len - auth1_length));
    const void* auth2_data = ctx.first();
    if (!ctx.enough_size(auth2_length))
        return deserialize_errc::incomplete_message;
    ctx.advance(auth2_length);

    // Auth plugin name
    err = deserialize(ctx, output.auth_plugin_name);
    if (err != deserialize_errc::ok)
        return err;

    // Compose auth_plugin_data
    output.auth_plugin_data.clear();
    output.auth_plugin_data.append(auth_plugin_data_part_1.data(), auth1_length);
    output.auth_plugin_data.append(
        auth2_data,
        auth2_length - 1
    );  // discard an extra trailing NULL byte

    return deserialize_errc::ok;
}

std::size_t boost::mysql::detail::serialization_traits<
    boost::mysql::detail::handshake_response_packet,
    boost::mysql::detail::serialization_tag::struct_with_fields>::
    get_size_(const serialization_context& ctx, const handshake_response_packet& value) noexcept
{
    std::size_t res = get_size(
                          ctx,
                          value.client_flag,
                          value.max_packet_size,
                          value.character_set,
                          value.username,
                          value.auth_response
                      ) +
                      23;  // filler
    if (ctx.get_capabilities().has(CLIENT_CONNECT_WITH_DB))
    {
        res += get_size(ctx, value.database);
    }
    res += get_size(ctx, value.client_plugin_name);
    return res;
}

inline void boost::mysql::detail::serialization_traits<
    boost::mysql::detail::handshake_response_packet,
    boost::mysql::detail::serialization_tag::struct_with_fields>::
    serialize_(serialization_context& ctx, const handshake_response_packet& value) noexcept
{
    serialize(
        ctx,
        value.client_flag,
        value.max_packet_size,
        value.character_set,
        string_fixed<23>{},
        value.username,
        value.auth_response
    );

    if (ctx.get_capabilities().has(CLIENT_CONNECT_WITH_DB))
    {
        serialize(ctx, value.database);
    }
    serialize(ctx, value.client_plugin_name);
}

inline boost::mysql::detail::deserialize_errc boost::mysql::detail::serialization_traits<
    boost::mysql::detail::auth_switch_request_packet,
    boost::mysql::detail::serialization_tag::struct_with_fields>::
    deserialize_(deserialization_context& ctx, auth_switch_request_packet& output) noexcept
{
    auto err = deserialize(ctx, output.plugin_name, output.auth_plugin_data);
    auto& auth_data = output.auth_plugin_data.value;
    // Discard an additional NULL at the end of auth data
    if (!auth_data.empty())
    {
        assert(auth_data.back() == 0);
        auth_data = auth_data.substr(0, auth_data.size() - 1);
    }
    return err;
}

#endif /* INCLUDE_BOOST_MYSQL_DETAIL_PROTOCOL_IMPL_HANDSHAKE_MESSAGES_IPP_ */
