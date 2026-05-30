//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_PROTOCOL_HANDSHAKE_MESSAGES_HPP
#define BOOST_MYSQL_DETAIL_PROTOCOL_HANDSHAKE_MESSAGES_HPP

#include <boost/mysql/detail/auxiliar/static_string.hpp>
#include <boost/mysql/detail/protocol/serialization.hpp>

namespace boost {
namespace mysql {
namespace detail {

// initial handshake
struct handshake_packet
{
    using auth_buffer_type = static_string<8 + 0xff>;

    // int<1>     protocol version     Always 10
    string_null server_version;
    std::uint32_t connection_id;
    auth_buffer_type auth_plugin_data;  // not an actual protocol field, the merge of two fields
    std::uint32_t capability_falgs;     // merge of the two parts - not an actual field
    std::uint8_t character_set;  // default server a_protocol_character_set, only the lower 8-bits
    std::uint16_t status_flags;  // server_status_flags
    string_null auth_plugin_name;

    template <class Self, class Callable>
    static void apply(Self& self, Callable&& cb)
    {
        // clang-format off
        std::forward<Callable>(cb)(
            self.server_version,
            self.connection_id,
            self.auth_plugin_data,
            self.capability_falgs,
            self.character_set,
            self.status_flags,
            self.auth_plugin_name
        );
        // clang-format on
    }
};

template <>
struct serialization_traits<handshake_packet, serialization_tag::struct_with_fields>
    : noop_serialize<handshake_packet>
{
    static inline deserialize_errc deserialize_(
        deserialization_context& ctx,
        handshake_packet& output
    ) noexcept;
};

// response
struct handshake_response_packet
{
    std::uint32_t client_flag;  // capabilities
    std::uint32_t max_packet_size;
    std::uint8_t character_set;
    // string[23]     filler     filler to the size of the handhshake response packet. All 0s.
    string_null username;
    string_lenenc auth_response;     // we require CLIENT_PLUGIN_AUTH_LENENC_CLIENT_DATA
    string_null database;            // only to be serialized if CLIENT_CONNECT_WITH_DB
    string_null client_plugin_name;  // we require CLIENT_PLUGIN_AUTH
    // CLIENT_CONNECT_ATTRS: not implemented

    template <class Self, class Callable>
    static void apply(Self& self, Callable&& cb)
    {
        std::forward<Callable>(cb
        )(self.client_flag,
          self.max_packet_size,
          self.character_set,
          self.username,
          self.auth_response,
          self.database,
          self.client_plugin_name);
    }
};

template <>
struct serialization_traits<handshake_response_packet, serialization_tag::struct_with_fields>
    : noop_deserialize<handshake_response_packet>
{
    static inline std::size_t get_size_(
        const serialization_context& ctx,
        const handshake_response_packet& value
    ) noexcept;

    static inline void serialize_(
        serialization_context& ctx,
        const handshake_response_packet& value
    ) noexcept;
};

// SSL request
struct ssl_request
{
    std::uint32_t client_flag;
    std::uint32_t max_packet_size;
    std::uint8_t character_set;
    string_fixed<23> filler;

    template <class Self, class Callable>
    static void apply(Self& self, Callable&& cb)
    {
        // clang-format off
        std::forward<Callable>(cb)(
            self.client_flag,
            self.max_packet_size,
            self.character_set,
            self.filler
        );
        // clang-format on
    }
};

// auth switch request
struct auth_switch_request_packet
{
    string_null plugin_name;
    string_eof auth_plugin_data;

    template <class Self, class Callable>
    static void apply(Self& self, Callable&& cb)
    {
        std::forward<Callable>(cb)(self.plugin_name, self.auth_plugin_data);
    }
};

// response
struct auth_switch_response_packet
{
    string_eof auth_plugin_data;

    template <class Self, class Callable>
    static void apply(Self& self, Callable&& cb)
    {
        std::forward<Callable>(cb)(self.auth_plugin_data);
    }
};

template <>
struct serialization_traits<auth_switch_request_packet, serialization_tag::struct_with_fields>
    : noop_serialize<auth_switch_request_packet>
{
    static inline deserialize_errc deserialize_(
        deserialization_context& ctx,
        auth_switch_request_packet& output
    ) noexcept;
};

// more data (like auth switch request, but for the same plugin)
struct auth_more_data_packet
{
    string_eof auth_plugin_data;

    template <class Self, class Callable>
    static void apply(Self& self, Callable&& cb)
    {
        std::forward<Callable>(cb)(self.auth_plugin_data);
    }
};

}  // namespace detail
}  // namespace mysql
}  // namespace boost

#include <boost/mysql/detail/protocol/impl/handshake_messages.ipp>

#endif /* INCLUDE_BOOST_MYSQL_DETAIL_PROTOCOL_HANDSHAKE_MESSAGES_HPP_ */
